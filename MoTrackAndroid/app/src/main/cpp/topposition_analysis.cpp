//
//  topposition.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/22/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
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

#include "topposition_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    //returns the square of the distance between two points
    double dist2BetweenPts(cv::Point p1, cv::Point p2) {
        cv::Point p3 = p1-p2;
        return p3.x*p3.x + p3.y*p3.y;
    }
    
    //returns the distance between two points
    double distBetweenPts(cv::Point p1, cv::Point p2) {
        return std::sqrt(dist2BetweenPts(p1,p2));
    }
    
    
    //param is returned in units of degrees
    double analyze_thumb_opposition(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        double param = PARAM_NO_HAND;
        
        distInfo d = get_distance_transform_adj(fgmask, true);
        double radius = d.dist.at<float>(d.maxLoc.y,d.maxLoc.x);
        
        if (displayExtraInfoAmount > 0) cv::circle(frame_with_mask, d.maxLoc/ratio, radius/ratio, CLR_MOTRACK_BLUE_4UC, 5);
        
        if (radius > 0 && d.maxLoc.y+radius < fgmask.rows) {
            param = PARAM_NOT_VALID;
            
            Rect fingerAndPalmRect(0, 0, fgmask.cols, d.maxLoc.y+radius); //goes up to bottom of "palm"
            //clone() is needed for Android, but not iOS (idk why) or part of the hand is blocked out
            Mat fingerAndPalm = fgmask(fingerAndPalmRect).clone();

            vector< vector<Point> > contours; // list of contour points
            vector<Vec4i> hierarchy;
            // find contours
            cv::findContours(fingerAndPalm, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

            double max_area = 0;
            int max_ct_ind = -1;
            for (int i = 0; i < contours.size(); i++) {
                double ct_area = cv::contourArea(contours[i]);
                if (ct_area > max_area) {
                    max_area = ct_area;
                    max_ct_ind = i;
                }
            }
            if (max_area/(fingerAndPalm.rows*fingerAndPalm.cols) < 0.05) { // prevent small objects from being detected
                if (displayExtraInfoAmount > 0 && contours[max_ct_ind].size() > 3) {
                    for(int i2=0; i2<contours[max_ct_ind].size(); i2++) {
                        contours[max_ct_ind][i2] = contours[max_ct_ind][i2]/ratio;
                    }

                    vector< vector<cv::Point> > hullPts(1);
                    cv::convexHull(Mat(contours[max_ct_ind]), hullPts[0], false);
                    cv::drawContours(frame_with_mask, hullPts, 0, Scalar(255,0,0,255), 4, 8, vector<Vec4i>(), 0, Point());
                }

                max_ct_ind = -1;
            }
            if (max_ct_ind != -1) {
                for(int i1=0; i1<contours.size(); i1++) {
                    for(int i2=0; i2<contours[i1].size(); i2++) {
                        contours[i1][i2] = contours[i1][i2]/ratio;
                    }
                }
                
                // create hull array for convex hull points
                vector< vector<int> > hullInds(1); //array of size 1 because will only do this for one contour
                vector< vector<cv::Point> > hullPts(1);
                cv::convexHull(Mat(contours[max_ct_ind]), hullInds[0], false); //If input for hull is vector of points will return points. If hull is vector of vector of ints, it will return indices.
                cv::convexHull(Mat(contours[max_ct_ind]), hullPts[0], false);
                
                //can draw hull, but we would need hull to be a vector< vector<Point> >
                if (displayExtraInfoAmount > 0) {
                    cv::drawContours(frame_with_mask, hullPts, 0, CLR_MOTRACK_BLUE_4UC, 4, 8, vector<Vec4i>(), 0, Point());

                    int padding = 25;
                    for (int i = 0; i < hullPts[0].size(); i++) {
                        int val = padding + (255-padding-padding)*(i+1.0)/hullPts[0].size();
                        cv::circle(frame_with_mask, hullPts[0][i], 5, Scalar(val,val,val,255), 4 );
                    }

                    /*
                    for (int i = 0; i < contours[max_ct_ind].size(); i++) {
                        int val = padding + (255-padding-padding)*(i+1.0)/contours[max_ct_ind].size();
                        cv::circle(frame_with_mask, contours[max_ct_ind][i], 5, Scalar(val,val,val,255), 4 );
                    }*/
                }
                
                if (contours[max_ct_ind].size() > 3 && hullInds[0].size() > 3) {
                    param = 0;
                    vector< vector<cv::Vec4i> >convDef(1);
                    cv::convexityDefects(contours[max_ct_ind], hullInds[0], convDef[0]);
                    
                    int maxdist = 0;
                    int maxDefectInd = -1;
                    int it=0;
                    for (Vec4i defect : convDef[0]) {
                        if ( abs(contours[0][defect[0]].x) < 2000 && abs(contours[0][defect[1]].x) < 2000 && abs(contours[0][defect[2]].x) < 2000 && abs(contours[0][defect[0]].y) < 2000 && abs(contours[0][defect[1]].y) < 2000 && abs(contours[0][defect[2]].y) < 2000 ) {
                            
                            //Note: defect[0] and defect[1] represent indices for previous and next contour points that touch the hull
                            //Note: defect[2] represents index for point with the distance furthest point from the convex hull
                            //Note: defect[3] is an int that is 256 times the distance point defects[2] is from the convex hull (is *256 so that it can be an int)
                            double distanceToPalm = distBetweenPts(contours[0][defect[2]], d.maxLoc/ratio);
                            //double metric = defect[3];
                            //double metric = defect[3]/distanceToPalm;
                            double metric = distanceToPalm<1.5*radius/ratio ? defect[3] : 0;
                            if (defect[3]/256.0 < 0.5*radius/ratio) metric = 0;

                            //The following block of code is needed because in Android, it counts a convexity defect between the last eleemnt and the first element
                            //(i.e. it creates a "convexity defect" that actually goes through the middle of the contour
                            //It doesn't happen in iOS for some reason.
                            // Note: only the first half of this "if" condition or has been empirically
                            // found to happen (i.e. evaluate to true), but let's test for both just in case
                            if ( (defect[0]==hullInds[0][0] && defect[1]==hullInds[0][hullInds[0].size()-1]) ||
                                 (defect[1]==hullInds[0][0] && defect[0]==hullInds[0][hullInds[0].size()-1]) ) {
                                metric = 0;
                            }
                            if (metric > maxdist) {
                                maxdist = metric;
                                maxDefectInd = it;
                            }
                        }
                        it++;

                    }
                    
                    if (maxDefectInd != -1) {
                        Vec4i defect = convDef[0][maxDefectInd];
                        Point center = contours[0][defect[2]];
                        //Point center = d.maxLoc/ratio;
                        
                        //law of cosines to get the angle, c^2 = a^2 + b^2 -2*a*b*cos(C)
                        double c2 = dist2BetweenPts(contours[0][defect[0]], contours[0][defect[1]]);
                        double a  =  distBetweenPts(contours[0][defect[0]], center);
                        double b  =  distBetweenPts(contours[0][defect[1]], center);
                        double cosC = (c2-a*a-b*b)/(-2*a*b);
                        param = acos(cosC)*180/M_PI; //convert radians to degrees
                        
                        if (displayExtraInfoAmount > 0) {
                            if (displayExtraInfoAmount > 1) cout << param << endl;

                            cv::line(frame_with_mask, contours[0][defect[0]], center, Scalar(0,   0, 255, 255), 3);
                            cv::line(frame_with_mask, contours[0][defect[1]], center, Scalar(0, 255, 255, 255), 3);
                            cv::circle(frame_with_mask, center, 5, Scalar(255,255,255,255), 2 );
                        }
                    }
                    
                    /*
                    it = 0;
                    for (Vec4i defect : convDef[0]) {
                        //can't figure out why, but sometimes defects is randomly super high (positive or negative) values, which can crash the app when trying to draw them with cv::line for example
                        //Example points: [0, 5] [1451292736, 1] [90, 120]; [1451292736, 1] [128, 112] [1470175312, 1]; [128, 112] [0, 0] [0, 0]; [0, 0] [-1362131800, 1] [0, 1441792]; [-1362131800, 1] [220, 108] [0, 5]; [220, 108] [212, 112] [216, 112]; [212, 112] [216, 124] [212, 124]; [216, 124] [1470146280, 1] [0, 0]; [1470146280, 1] [0, 1] [-1, -2097153]; [0, 1] [272, 131112] [360320196, -2147483648]; [0, 0] [0, 0] [0, 0]; [0, 0] [0, 0] [0, 0]; [0, 0] [-2085707364, 1] [512, 1];
                        //because of this, we need this if statement
                        if ( abs(contours[0][defect[0]].x) < 2000 && abs(contours[0][defect[1]].x) < 2000 && abs(contours[0][defect[2]].x) < 2000 && abs(contours[0][defect[0]].y) < 2000 && abs(contours[0][defect[1]].y) < 2000 && abs(contours[0][defect[2]].y) < 2000 ) {
                            
                            if (maxDefectInd == it || true) {
                                //law of cosines to get the angle, c^2 = a^2 + b^2 -2*a*b*cos(C)
                                double c2 = dist2BetweenPts(contours[0][defect[0]], contours[0][defect[1]]);
                                double a  =  distBetweenPts(contours[0][defect[0]], contours[0][defect[2]]);
                                double b  =  distBetweenPts(contours[0][defect[1]], contours[0][defect[2]]);
                                double cosC = (c2-a*a-b*b)/(-2*a*b);
                                param = acos(cosC)*180/M_PI; //convert radians to degrees
                                cout << param << endl;

                                if (maxDefectInd == it) {
                                    cv::line(frame_with_mask, contours[0][defect[0]], contours[0][defect[2]], Scalar(0,   0, 255, 255), 3);
                                    cv::line(frame_with_mask, contours[0][defect[1]], contours[0][defect[2]], Scalar(0, 255, 255, 255), 3);
                                    cv::circle(frame_with_mask, contours[0][defect[2]], 5, Scalar(255,255,255,255), 2 );
                                } else if (displayExtraInfoAmount > 1) {
                                    cv::line(frame_with_mask, contours[0][defect[0]], contours[0][defect[2]], Scalar(0,   0, 100, 255), 3);
                                    cv::line(frame_with_mask, contours[0][defect[1]], contours[0][defect[2]], Scalar(0, 100, 100, 255), 3);
                                    cv::circle(frame_with_mask, contours[0][defect[2]], 5, Scalar(150,150,150,255), 4 );
                                }

                            }
                            
                        }
                        it++;
                    }*/
                    //cout << maxdist/256.0 << endl;
                    //cout << endl;
                }
                
                
                
            }
        }
        
        if (displayExtraInfoAmount > 2) {
            char buf[125];
            snprintf(buf, sizeof(buf), "Param: %.2f", param);
            writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.30));
        }
        
        
        return param;
        
        
    }
    
    
#ifdef EXTERN_C
}
#endif
