//
//  hook_fist_caterpillar.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 5/23/19.
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

#include "hook_fist_caterpillar.h"
#include "hook_fist_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    const double CATERPILLAR_STRETCHED_OUT_PARAM = 1.7; //these are constant regardless of the ROM, only whether or not it flashes green/gives feedback is what changes
    const double CATERPILLAR_SCRUNCHED_UP_PARAM = 0.7; //these are constant regardless of the ROM, only whether or not it flashes green/gives feedback is what changes
    const double CATERPILLAR_HEAD_SEGMENT_SCALE = 1.5; //relative to the other sizes
    const double CATERPILLAR_SEGMENT_CT = 8;
    const double ANTENNA_LENGTH_RATIO = 0.5; //ratio of variable distance (the space between the segment centers). This is the length in one direction (x or y) so the actual distance is this times sqrt(2)
    
    cv::Mat caterpillarGameScenery;
    
    cv::Mat caterpillarGameSceneryOrig;
    
    bool caterpillarImagesInitialized = false;

#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setCaterpillarGameScenery(JNIEnv *env, jobject, long addr) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, caterpillarGameSceneryOrig, cv::COLOR_BGRA2RGBA);
        caterpillarImagesInitialized = false;
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setCaterpillarGameScenery(cv::Mat picture) {
        if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
        caterpillarGameSceneryOrig = picture;
        caterpillarImagesInitialized = false;
    }
    
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */

    
    size_t caterpillarGameFrameCt = 0;
    
    double paramToDisplayAngle(double param) {
        return 90*(1-(param-CATERPILLAR_SCRUNCHED_UP_PARAM)/(CATERPILLAR_STRETCHED_OUT_PARAM-CATERPILLAR_SCRUNCHED_UP_PARAM));
    }
    
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_caterpillar_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        if (!caterpillarImagesInitialized) {
            if (caterpillarGameSceneryOrig.data == NULL) {
                //light green
                caterpillarGameSceneryOrig = cv::Mat(25,25, CV_8UC4, cv::Scalar(0,255,0, 255)); //beige
                //caterpillarGameSceneryOrig( cv::Scalar(167, 200, 129, 255) );
            }
            
            if (whichHand == Hand::LEFT) {
                cv::flip(caterpillarGameSceneryOrig, caterpillarGameSceneryOrig, +1);
                caterpillarGameScenery.data = NULL;
            }
            caterpillarGameFrameCt = 0;
            caterpillarImagesInitialized = true;
        }
        
        
        
        Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(0, textWriteRectangle.height, frame_with_mask.cols, frame_with_mask.rows-textWriteRectangle.height)) : frame_with_mask;
        
        
        double param = analyze_hook_fist(fgmask, frame_with_mask, dir, ratio);
        double displParam = param;

        caterpillarGameFrameCt = std::min(caterpillarGameFrameCt, paramHistory.size()); //necessary if paramHistory is cleared more often than caterpillarGameFrameCt is because of changes to the app (like it was when I first added this; it's safer to change it here than constantly update caterpillarGameFrameCt)

        //Filter parameter to make graphics less jumpy
        int sum_size = 2; //amount of elements in filter; this does not include the current param
        if (caterpillarGameFrameCt >= sum_size && !isParamNoHand(param)) {
            double sum = 0;
            int effective_sum_size = 0; //hopefully, will end up being sum_size, but not necessarily; safer to count upwards than downwards to prevent a bug where the for loop ends prematurely (i.e. paramHistory is shorter than anticipated)
            unsigned long i = 0;
            
            /* This old method of iterating was working weird (sometimes paramHistory.size() was higher than expected with later values just giving zeros.
            for (i = 1; i <= sum_size; i++) {
                double thisParam = paramHistory[paramHistory.size()-i];
                //double thisParam = paramHistory.at(paramHistory.size()-i);
                //cout << thisParam << " ";
                if (isParamNoExerciseDone(thisParam)) {
                    effective_sum_size--;
                } else sum += thisParam;
            }*/
            
            //crbegin, crend iterate in reverse (start at the last element in paramHistory aka the most recent element in paramHistory, and then go backwards in time)
            for (auto it = paramHistory.crbegin(); it != paramHistory.crend(); ++it) {
                double thisParam = *it;
                if (isParamNoExerciseDone(thisParam)) {
                } else {
                    sum += thisParam;
                    effective_sum_size++;
                }
                
                if (++i >= sum_size) break;
            }
            
            if (effective_sum_size > 0) {
                if (isParamNoExerciseDone(param)) { //if current param is no exercise done, don't count it
                    displParam = sum/effective_sum_size;
                } else {
                    displParam = (param+sum)/(effective_sum_size+1);
                }
            }
        }
        param = displParam; //decides whether or not there is a feedback loop where the filtered value is used for future filters
        caterpillarGameFrameCt++;
        
        
        
        if (caterpillarGameSceneryOrig.data != NULL) {
            cv::resize(caterpillarGameSceneryOrig, caterpillarGameScenery, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, INTER_LINEAR);
            
            /*double diameter = frame_with_mask_draw.cols/10.0;
            Point center = Point(0*diameter, diameter*3);
            for (int i=1; i< 9; i++) {
                center = Point(i*diameter, diameter*3); //times 3 so that there is enough space on top when fully vertically spaced.
                cv::circle(caterpillarGameScenery, center, diameter/2, CLR_BLACK_4UC, -1);
            }*/
            
            double angle_degs = paramToDisplayAngle(displParam);
            if (isParamNoExerciseDone(displParam)) {
                angle_degs = 0;
                //make it the last available value
                /*
                int max_size = 10; unsigned long i = 0;
                for (auto it = paramHistory.crbegin(); it != paramHistory.crend(); ++it) {
                    double thisParam = *it;
                    if (isParamNoExerciseDone(thisParam)) {
                        
                    } else {
                        angle_degs = paramToDisplayAngle(thisParam);
                        break;
                    }
                    
                    if (++i >= max_size) break;
                }*/
                
            }
            
            if (angle_degs > 90) angle_degs = 90;
            else if (angle_degs < 0) angle_degs = 0;
            
            double distance = frame_with_mask_draw.cols/(CATERPILLAR_SEGMENT_CT+2.0); //distance between center of one segment and the next, all should be the distance between the caterpillar first/last segment center and the phone side edge if the caterpillar is fully stretched
            double padding = distance;
            
            double angle_degs_limit = paramToDisplayAngle(motionsMap.motions[dir].param);
            double limit_loc; //this represents where the desired parameter is marked to be (a line and an arrow will be drawn here for user feedback)
            if (whichHand == Hand::LEFT) {
                limit_loc = padding+3*distance+CATERPILLAR_SEGMENT_CT/2*distance*cos(M_PI/180*angle_degs_limit)+CATERPILLAR_HEAD_SEGMENT_SCALE*distance/2;
                
            } else {
                padding = frame_with_mask_draw.cols-padding-3*distance-CATERPILLAR_SEGMENT_CT/2*distance*cos(M_PI/180*angle_degs);
                limit_loc = frame_with_mask_draw.cols-distance-3*distance-CATERPILLAR_SEGMENT_CT/2*distance*cos(M_PI/180*angle_degs_limit)-CATERPILLAR_HEAD_SEGMENT_SCALE*distance/2;
                
            }
            
            //set up code for drawing arrows
            Scalar outlineClr = CLR_BLACK_4UC, arrowClr = CLR_MOTRACK_BLUE_4UC, markClr = CLR_MOTRACK_BLUE_4UC, markLineClr = CLR_BLACK_4UC;
            double tipLength = 0.3;
            double arrowLength = frame_with_mask.cols*0.175;
            double markStart = distance/2;
            double markHt = distance*3-distance/2*CATERPILLAR_HEAD_SEGMENT_SCALE-distance*ANTENNA_LENGTH_RATIO-markStart*2;
            double arrowShift = distance/8;
            if ( motionsMap.motions[dir].comparisonMethod == COMPARE_GREATER_THAN) {
                arrowLength = -arrowLength;
                arrowShift = -arrowShift;
            }
            if (whichHand == Hand::LEFT) {
                arrowLength = -arrowLength;
                arrowShift = -arrowShift;
            }
            bool isParamBeingCompleted = checkIfParamCompleted(dir,param);
            
            if (!isParamNoExerciseDone(param)) {
                if (isParamBeingCompleted) {
                    markLineClr  = CLR_FEEDBACK_GREEN_4UC;
                } else {
                    markLineClr  = CLR_FEEDBACK_RED_4UC;
                }
            }
            

            double center_x = padding-distance, center_y = distance*3; //times 3 so that there is enough space on top when fully vertically spaced.
            
            vector<double> center_xs;
            vector<double> center_ys;
            center_xs.reserve(CATERPILLAR_SEGMENT_CT); //not the same as defining using the (length) constructor (example: vector<int> center_xs(8))
            center_ys.reserve(CATERPILLAR_SEGMENT_CT);
            for (int i=0; i < CATERPILLAR_SEGMENT_CT; i++) {
                if (i==0 || i==1 || i==CATERPILLAR_SEGMENT_CT/2 || i==CATERPILLAR_SEGMENT_CT-1) {
                    center_x += distance;
                    center_y += 0;
                } else {
                    center_x += distance*cos(M_PI/180*angle_degs);
                    if (i<CATERPILLAR_SEGMENT_CT/2) center_y -= distance*sin(M_PI/180*angle_degs);
                    else center_y += distance*sin(M_PI/180*angle_degs);
                }
                center_xs.push_back(center_x);
                center_ys.push_back(center_y);
                
            }
            
            
            //Start the graphics
            //draw line showing where the param requirement is, make the color on the outside for effect
            //cv::line(caterpillarGameScenery, cv::Point(limit_loc,markStart), cv::Point(limit_loc,distance*4.5), markLineClr, distance/8);
            //cv::line(caterpillarGameScenery, cv::Point(limit_loc,markStart), cv::Point(limit_loc,distance*4.5), outlineClr, distance/18);
            drawDashes(caterpillarGameScenery, cv::Point(limit_loc, markStart), 90,
                       distance * 1.0 / 4, distance * 1.0 / 4, 8, markLineClr, distance / 8);
            drawDashes(caterpillarGameScenery, cv::Point(limit_loc, markStart), 90,
                       distance * 1.0 / 4, distance * 1.0 / 4, 8, outlineClr, distance / 18);
            

            for (int i=7; i >= 0; i--) {
                double centerX = center_xs[i];
                double centerY = center_ys[i];
                double diameter = i==0 ? distance*CATERPILLAR_HEAD_SEGMENT_SCALE : distance; //diameter of the body segment about to be drawn
                double thickness = std::max(1.0,distance/18);
                
                //get color for body segment
                Scalar segmentColor;
                if (isParamNoExerciseDone(displParam)) {
                    segmentColor = CLR_MOTRACK_GRAY_4UC;
                } else {
                    if (i%2==1) segmentColor = cv::Scalar(180,200,129, 255);
                    else segmentColor = cv::Scalar(167,200,129, 255);
                    //segmentColor = Scalar(255, 174, 66,255);
                }
                
                //draw body segment
                cv::circle(caterpillarGameScenery, Point(centerX,centerY), diameter/2, segmentColor, -1); //draw segment
                cv::circle(caterpillarGameScenery, Point(centerX,centerY), diameter/2, outlineClr, thickness); //draw segment outline
                
                if (i==0) { //draw face/antennae
                    //draw antennae
                    cv::line(caterpillarGameScenery, Point(centerX,centerY-diameter/2), Point(centerX-distance*ANTENNA_LENGTH_RATIO,centerY-diameter/2-distance*ANTENNA_LENGTH_RATIO), outlineClr, thickness);
                    cv::line(caterpillarGameScenery, Point(centerX,centerY-diameter/2), Point(centerX+distance*ANTENNA_LENGTH_RATIO,centerY-diameter/2-distance*ANTENNA_LENGTH_RATIO), outlineClr, thickness);
                    
                    //draw eyes
                    Point leftEyeCenter  = Point(centerX-diameter/6,centerY-diameter/6);
                    Point rightEyeCenter = Point(centerX+diameter/6,centerY-diameter/6);
                    Scalar eyePupilClr = Scalar(65,48,220,255);
                    if (isParamBeingCompleted) {
                        cv::ellipse(caterpillarGameScenery,  leftEyeCenter, cv::Size(std::max(diameter/8,1.0),std::max(diameter/6,1.0)), 0, 0, 360, CLR_WHITE_4UC, -1); //left eye inside
                        cv::ellipse(caterpillarGameScenery, rightEyeCenter, cv::Size(std::max(diameter/8,1.0),std::max(diameter/6,1.0)), 0, 0, 360, CLR_WHITE_4UC, -1); //right eye inside
                        cv::ellipse(caterpillarGameScenery,  leftEyeCenter, cv::Size(std::max(diameter/8,1.0),std::max(diameter/6,1.0)), 0, 0, 360, outlineClr, thickness); //left eye outline
                        cv::ellipse(caterpillarGameScenery, rightEyeCenter, cv::Size(std::max(diameter/8,1.0),std::max(diameter/6,1.0)), 0, 0, 360, outlineClr, thickness); //right eye outline
                        
                        cv::circle(caterpillarGameScenery,  leftEyeCenter, std::max(diameter/18,1.0), eyePupilClr, -1); //left eye pupil
                        cv::circle(caterpillarGameScenery, rightEyeCenter, std::max(diameter/18,1.0), eyePupilClr, -1); //right eye pupil
                        
                    } else {
                        cv::circle(caterpillarGameScenery,  leftEyeCenter, std::max(diameter/8,1.0), CLR_WHITE_4UC, -1); //left eye inside
                        cv::circle(caterpillarGameScenery, rightEyeCenter, std::max(diameter/8,1.0), CLR_WHITE_4UC, -1); //right eye inside
                        cv::circle(caterpillarGameScenery,  leftEyeCenter, std::max(diameter/8,1.0), outlineClr, thickness); //left eye outline
                        cv::circle(caterpillarGameScenery, rightEyeCenter, std::max(diameter/8,1.0), outlineClr, thickness); //right eye outline
                        
                        cv::circle(caterpillarGameScenery,  leftEyeCenter, std::max(diameter/24,1.0), eyePupilClr, -1); //left eye pupil
                        cv::circle(caterpillarGameScenery, rightEyeCenter, std::max(diameter/24,1.0), eyePupilClr, -1); //right eye pupil
                    }

                    
                    //draw mouth
                    if (isParamBeingCompleted) cv::ellipse(caterpillarGameScenery, Point(centerX,centerY+diameter/8), cv::Size(diameter*0.3,diameter/4), 0, 0, 180, CLR_WHITE_4UC, -1);
                    cv::ellipse(caterpillarGameScenery, Point(centerX,centerY+diameter/8), cv::Size(diameter*0.3,diameter/4), 0, 0, 180, outlineClr, thickness);
                    if (isParamBeingCompleted) {
                        //cv::line(caterpillarGameScenery, Point(centerX-diameter/4,centerY+diameter/8), Point(centerX+diameter/4,centerY+diameter/8), outlineClr, light_thickness);
                        cv::ellipse(caterpillarGameScenery, Point(centerX,centerY+diameter/8), cv::Size(diameter*0.3,diameter/8), 0, 0, 180, segmentColor, -1);
                        cv::ellipse(caterpillarGameScenery, Point(centerX,centerY+diameter/8), cv::Size(diameter*0.3,diameter/8), 0, 0, 180, outlineClr, thickness);
                    }

                }
            }
            
            
            //draw feedback arrows
            cv::arrowedLine(caterpillarGameScenery, cv::Point(limit_loc+arrowShift,markHt/2+markStart), cv::Point(limit_loc+arrowLength,markHt/2+markStart), outlineClr, distance/2, 8, 0, tipLength);
            cv::arrowedLine(caterpillarGameScenery, cv::Point(limit_loc+arrowShift,markHt/2+markStart), cv::Point(limit_loc+arrowLength,markHt/2+markStart),   arrowClr, distance/4, 8, 0, tipLength);
            cv::line(caterpillarGameScenery, cv::Point(limit_loc,markStart), cv::Point(limit_loc,markStart+markHt), outlineClr, distance/2);
            cv::line(caterpillarGameScenery, cv::Point(limit_loc,markStart), cv::Point(limit_loc,markStart+markHt), markClr, distance/4);
            
            
            
            
            // for text that will be written on top/side
            if (shouldWriteOnIm) {
                //Don't need to draw the rectangle since copyMakeBorder already does the same effect
                cv::copyMakeBorder(caterpillarGameScenery, caterpillarGameScenery, textWriteRectangle.height, 0, 0, 0, BORDER_CONSTANT, textWriteRectangleBackground);
                
                cv::line(caterpillarGameScenery, cv::Point(0, textWriteRectangle.height), cv::Point(caterpillarGameScenery.cols, textWriteRectangle.height), CLR_BLACK_4UC, 5);
                
            }
            
            
            Mat bgmask;
            bitwise_not(fgmask, bgmask);
            cv::resize(bgmask, bgmask, frame_with_mask.size(), 0, 0, INTER_NEAREST );
            bitwise_and(caterpillarGameScenery, caterpillarGameScenery, frame_with_mask, bgmask);
            
        }
        
        
        return param;
        
    }
    
    
#ifdef EXTERN_C
}
#endif
