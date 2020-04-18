//
//  fabduction_analysis.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/14/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//
// NOTE: "fabduction" is short for finger abduction
//


//The calibration import must always be the first import in order to make use of device-dependent preprocessor directives
#include "calibration.h"
//#include "all_access_vars.h"


//device specific includes/imports
#if MY_OS==ANDROID_OS
//Below are the imports for the android version
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
//#include "Eigen/Dense"

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
//Below are the imports for the iOS version
#include <stdio.h>

#endif


// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cmath>

#include "fabduction_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    double analyze_fabduction(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        // if displayExtraInfoAmount is 1, 3, 5, 7, etc

        if (displayExtraInfoAmount > 0 && displayExtraInfoAmount % 2 ==1) {
#if (MY_OS==ANDROID_OS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
            return analyze_fabduction_unrotated(fgmask, frame_with_mask, dir, ratio);
#elif MY_OS==IOS
            return analyze_fabduction_with_rotation_allowed(fgmask, frame_with_mask, dir, ratio);
#endif
            
        } else {
#if (MY_OS==ANDROID_OS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
            return analyze_fabduction_with_rotation_allowed(fgmask, frame_with_mask, dir, ratio);
#elif MY_OS==IOS
            return analyze_fabduction_unrotated(fgmask, frame_with_mask, dir, ratio);
#endif
        }
        
        
    }
    
    //unrotated is the original version of the algorith. It works but required the hand to be straight, not rotated.
    double analyze_fabduction_unrotated(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        double param = PARAM_NO_HAND;

        if (cv::countNonZero(fgmask)*1.0/(fgmask.cols*fgmask.rows) < 0.05) {
            param = PARAM_NO_HAND;
            
        } else {
            param = PARAM_NOT_VALID;
            distInfo d = get_distance_transform(fgmask);
            double radius = (d.dist.at<float>(d.maxLoc.y,d.maxLoc.x));
            if (displayExtraInfoAmount > 1) {
                cv::circle(frame_with_mask, d.maxLoc/ratio, radius/ratio, CLR_MOTRACK_BLUE_4UC, 5);
            }
            
            if (d.maxLoc.y - radius*1.5 < 0) param = PARAM_NOT_VALID;
            //else if (d.maxLoc.y + radius >= fgmask.rows) param = NO_EXERCISE_DONE;
            else {
                int fingerRectYStart = std::max(d.maxLoc.y - radius*3.5, 0.0); //assume anything past 3.5 radius lengths is noise, not finger
                const int fingerRectWd = fgmask.cols;
                int fingerRectHt  = d.maxLoc.y - radius*0.5 - fingerRectYStart; //have the top of the rect end a quarterway into the circle in case the hand is slightly rotated, and the bottom part of the fingers are in the circle
                Rect fingerRect(0, fingerRectYStart, fingerRectWd, fingerRectHt);
                Mat fingerMat = fgmask(fingerRect);
                if (displayExtraInfoAmount > 1) {
                    Rect fingerRectDisp(0, fingerRectYStart/ratio, fgmask.cols/ratio, fingerRectHt/ratio);
                    cv::rectangle(frame_with_mask, fingerRectDisp, CLR_MOTRACK_GRAY_4UC, 10);
                }
                
                int totNonZero = cv::countNonZero(fingerMat);
                //to make the algorithm more robust, use a percentile based system instead of finding the absolute leftmost and absolute rightmost points. This is especially good for when the thumb accidentally gets placed in the finger region (this is more likely to happen when the hand is rotated a bit)
                const double percentLeft  = whichHand == Hand::LEFT ? 0.05 : 0.20; //extra buffer for the side the thumb is on
                const double percentRight = whichHand == Hand::LEFT ? 0.20 : 0.05;
                //int hist[90] = { };
                int nonZeroCt = 0;
                int leftMark = -1;
                int rightMark = -1;
                for (int col_it = 0; col_it < fingerMat.cols; col_it++) {
                    //const uchar* psrc_col  =  src.ptc(col_it);
                    Mat col = fingerMat.col(col_it);
                    int colNonzero = cv::countNonZero(col);
                    if (nonZeroCt >= percentLeft*totNonZero) {
                        if (leftMark == -1) leftMark = col_it; //leftMark is the first col from the left that percentLeft or more of the pixels are on the left
                    }
                    if (nonZeroCt >= (1-percentRight)*totNonZero) {
                        if (rightMark == -1) rightMark = col_it; //rightMark is the first col from the right that percentRight or more of the pixels are on the right (aka first col from left that 1-percentRight are on the left)
                    }
                    nonZeroCt = nonZeroCt + colNonzero;
                }
                if (leftMark == -1 || rightMark == -1) param = PARAM_NOT_VALID;
                else {
                    //Rasoning for following param formula:
                    //divide by 1-percentLeft-percentRight to normalize it the fact that we took certain percentiles of the x distribution histogram instead of absolute left and absolute right
                    //divide by 2*radius to normalize the fingers width to the palm width
                    //subtract by 1 to be centered around 0 to match other exercise parameters
                    param = (rightMark - leftMark)*1.0/(2*radius*(1-percentLeft-percentRight)) - 1;
                    if (displayExtraInfoAmount > 1) {
                        Rect foundFingerDisp(leftMark/ratio, fingerRectYStart/ratio, (rightMark-leftMark)/ratio, fingerRectHt/ratio);
                        cv::rectangle(frame_with_mask, foundFingerDisp, CLR_MOTRACK_BLUE_4UC, 5);
                        
                        char buf[125];
                        snprintf(buf, sizeof(buf), "Param: %.2f", param);
                        writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.70));
                    }
                    
                }
            }
            
        }
        

        return param;
    }
    
    
    double old_orientation = 90; //M_PI/2;
    double analyze_fabduction_with_rotation_allowed(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        distInfo d = get_distance_transform(fgmask);
        double radius = (d.dist.at<float>(d.maxLoc.y,d.maxLoc.x));
        if (displayExtraInfoAmount > 1) {
            cv::circle(frame_with_mask, d.maxLoc/ratio, radius/ratio, CLR_MOTRACK_BLUE_4UC, 5);
        }
        
        double param = PARAM_NOT_VALID;
        if (cv::countNonZero(fgmask)*1.0/(fgmask.cols*fgmask.rows) < 0.05) param = PARAM_NO_HAND;
        else if (d.maxLoc.y - radius*1.5 < 0) param = PARAM_NOT_VALID;
        else if (d.maxLoc.y + radius >= fgmask.rows) param = PARAM_NOT_VALID;
        else {
            double fingerRectYStart = std::max(d.maxLoc.y - radius*3.5, 0.0); //assume anything past 3.5 radius lengths is noise, not finger
            const int fingerRectWd = fgmask.cols;
            int fingerRectHt  = std::min(fgmask.rows-fingerRectYStart, d.maxLoc.y - radius*0.5 - fingerRectYStart); //have the top of the rect end a quarterway into the circle in case the hand is slightly rotated, and the bottom part of the fingers are in the circle
             int fingerAndPalmRectHt  = std::min(fgmask.rows-fingerRectYStart, d.maxLoc.y + radius - fingerRectYStart);
            Rect fingerRect(0, fingerRectYStart, fingerRectWd, fingerRectHt);
            //Mat fingerMat = fgmask(fingerRect);
            Mat fingerAndPalmMat = fgmask(Rect(0, fingerRectYStart, fingerRectWd, fingerAndPalmRectHt));

            
            
            //get axis using method of least second moments (from Dr. Reiter's lecture 3)
            /*Moments mom = cv::moments(fgmask, true); //2nd argument is whether binary matrix
            double xp =  mom.m10/mom.m00; //center x
            double yp =  mom.m01/mom.m00; //center y
            //double a =   mom.m20-xp*mom.m10;
            //double b = 2*mom.m11-xp*mom.m01; //could also be 2*mom.m11-yp*mom.m10
            //double c =   mom.m02-yp*mom.m01;
            
            double a = mom.m20-mom.m00*xp*yp;
            double b = 2*mom.m11-mom.m00*(xp*xp+yp*yp);
            double c = mom.m02-mom.m00*yp*yp;
            
            double theta1 = a==c ? 0 : atan2(b, a-c)/2; //axis of elongation
            double theta2 = theta1+M_PI/2; //perpendicular to axis of elongation
            double theta = theta1;
            double E1 = a*sin(theta)*sin(theta)-b*sin(theta)*cos(theta)+c*cos(theta)*cos(theta);
            theta = theta2;
            double E2 = a*sin(theta)*sin(theta)-b*sin(theta)*cos(theta)+c*cos(theta)*cos(theta);
            cout << E1/E2 << " " << theta1*180/M_PI << endl;*/
            
            //Mat matForOrientation = fingerAndPalmMat;
            //for whatever reason, in Android, it crashes sometimes and when it doens't crash, it draws out the fingers from fgmask. Unsure why only in Android or why this happens, but doing a .clone() appears to fix it. Untested it on computer.
#if (MY_OS==ANDROID_OS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
            Mat matForOrientation = fgmask(fingerRect).clone();
#elif MY_OS==IOS
            Mat matForOrientation = fgmask(fingerRect);
#endif
            vector<vector<cv::Point> > contours;
            vector<cv::Vec4i> hierarchy;
            cv::findContours(matForOrientation, contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
            double max_area = 0;
            int max_ct_ind = -1;
            for (int i = 0; i < contours.size(); i++) {
                double ct_area = cv::contourArea(contours[i]);
                if (ct_area > max_area) {
                    max_area = ct_area;
                    max_ct_ind = i;
                }
            }
            if (max_area/(matForOrientation.rows*matForOrientation.cols) < 0.05) { // prevent small objects from being detected
                vector<cv::Point> empty;
                max_ct_ind = -1;
                //return empty; //cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
            }
            if (max_ct_ind != -1) {
                /* double vecx, vecy;
                std::tie(vecx, vecy) = getOrientation(contours[max_ct_ind], matForOrientation);
                double orientation = atan2(vecy, vecx);
                
                 if ( orientation < 0) orientation = orientation + M_PI;
                 else if (orientation > M_PI) orientation = orientation - M_PI;
                 if ( orientation*180/M_PI < 60) orientation = 60*M_PI/180;
                 else if ( orientation*180/M_PI > 120) orientation = 120*M_PI/180;*/
                //if ( abs(orientation-old_orientation)*180/M_PI < 10) orientation = old_orientation;
                //else old_orientation = orientation; */
                
                RotatedRect rotatedRect = minAreaRect( Mat(contours[max_ct_ind]) );
                double vecx = rotatedRect.size.width;
                double vecy = rotatedRect.size.height;
                double orientation = rotatedRect.angle + 90;
                //rotatedRect angle is between [-90,0) and height is not always greater than width
                //below is a great explanation
                if(rotatedRect.size.width < rotatedRect.size.height){
                    orientation = orientation;// + 180;
                } else {
                    orientation = orientation + 90;
                }
                
                
                if (displayExtraInfoAmount > 1) {
                    cout << (int) orientation << " x: " << vecx << " y:" << vecy << endl;
                }
                
                Mat fgmaskRotated;
                double degreesRot = orientation - 90; //in degrees, not radians. Positive values mean counterclockwise.
                Mat rotMat = getRotationMatrix2D(d.maxLoc, degreesRot, 1.0);
                warpAffine(fgmask, fgmaskRotated, rotMat,
                           Size(fgmask.cols, fgmask.rows));
                Mat fingerMat = fgmaskRotated(fingerRect);
                
                int totNonZero = cv::countNonZero(fingerMat);
                //to make the algorithm more robust, use a percentile based system instead of finding the absolute leftmost and absolute rightmost points. This is especially good for when the thumb accidentally gets placed in the finger region (this is more likely to happen when the hand is rotated a bit)
                const double percentLeft  = whichHand == Hand::LEFT ? 0.05 : 0.20; //extra buffer for the side the thumb is on
                const double percentRight = whichHand == Hand::LEFT ? 0.20 : 0.05;
                //int hist[90] = { };
                int nonZeroCt = 0;
                int leftMark = -1;
                int rightMark = -1;
                for (int col_it = 0; col_it < fingerMat.cols; col_it++) {
                    //const uchar* psrc_col  =  src.ptc(col_it);
                    Mat col = fingerMat.col(col_it);
                    int colNonzero = cv::countNonZero(col);
                    if (nonZeroCt >= percentLeft*totNonZero) {
                        if (leftMark == -1) leftMark = col_it; //leftMark is the first col from the left that percentLeft or more of the pixels are on the left
                    }
                    if (nonZeroCt >= (1-percentRight)*totNonZero) {
                        if (rightMark == -1) rightMark = col_it; //rightMark is the first col from the right that percentRight or more of the pixels are on the right (aka first col from left that 1-percentRight are on the left)
                    }
                    nonZeroCt = nonZeroCt + colNonzero;
                }
                
                if (leftMark == -1 || rightMark == -1) param = PARAM_NOT_VALID;
                else {
                    //Rasoning for following param formula:
                    //divide by 1-percentLeft-percentRight to normalize it the fact that we took certain percentiles of the x distribution histogram instead of absolute left and absolute right
                    //divide by 2*radius to normalize the fingers width to the palm width
                    //subtract by 1 to be centered around 0 to match other exercise parameters
                    param = (rightMark - leftMark)*1.0/(2*radius*(1-percentLeft-percentRight)) - 1;
                    if (displayExtraInfoAmount > 1) {
                        /*Rect foundFingerDisp(leftMark/ratio, fingerRectYStart/ratio, (rightMark-leftMark)/ratio, fingerRectHt/ratio);
                        cv::rectangle(frame_with_mask, foundFingerDisp, CLR_MOTRACK_BLUE_4UC, 5);*/
                        
                        Mat rotMatForw = getRotationMatrix2D(d.maxLoc, degreesRot, 1.0);
                        warpAffine(frame_with_mask, frame_with_mask, rotMatForw,
                                   Size(frame_with_mask.cols, frame_with_mask.rows));
                        
                        Rect fingerRectDisp(0, fingerRectYStart/ratio, fgmask.cols/ratio, fingerRectHt/ratio);
                        cv::rectangle(frame_with_mask, fingerRectDisp, CLR_MOTRACK_GRAY_4UC, 10);
                        Rect foundFingerDisp(leftMark/ratio, fingerRectYStart/ratio, (rightMark-leftMark)/ratio, fingerRectHt/ratio);
                        cv::rectangle(frame_with_mask, foundFingerDisp, CLR_MOTRACK_BLUE_4UC, 5);
                        
                        Mat rotMatRev = getRotationMatrix2D(d.maxLoc, -degreesRot, 1.0);
                        warpAffine(frame_with_mask, frame_with_mask, rotMatRev,
                                   Size(frame_with_mask.cols, frame_with_mask.rows));
                    }
                    
                    
                    if (displayExtraInfoAmount > 1) {
                        char buf[125];
                        snprintf(buf, sizeof(buf), "Param: %.2f", param);
                        writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.80));
                        
                        //char buf2[125];
                        //snprintf(buf, sizeof(buf), "orient: %.2f", orientation);
                        writeText(frame_with_mask, "orient: " + std::to_string((int) orientation ), -1, int(frame_with_mask.rows * 0.15));
                        
                        char buf3[125];
                        if (vecx < 0) snprintf(buf, sizeof(buf), "x: %.2f", vecx);
                        else snprintf(buf, sizeof(buf), "y:  %.2f", vecx);
                        writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.3));
                        
                        char buf4[125];
                        if (vecy < 0) snprintf(buf, sizeof(buf), "x: %.2f", vecy);
                        else snprintf(buf, sizeof(buf), "x:  %.2f", vecy);
                        writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.5));
                    }
                    
                }
            }
            
            
            
        }
        
        return param;
    }

#ifdef EXTERN_C
}
#endif
