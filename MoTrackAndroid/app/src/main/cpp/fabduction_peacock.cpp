//
//  fabduction_peacock.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/21/19.
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

#include "fabduction_peacock.h"
#include "fabduction_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    const double PEACOCK_BODY_HEIGHT = 0.25; // measured as the height of bullseye image
    const double PEACOCK_BODY_CENTER_X_LOC = 0.5;
    const double PEACOCK_BODY_CENTER_Y_LOC = 0.28;
    const double PEACOCK_WITHIN_CENTER_X_LOC = 245.0/394; //rotation point x for feathers relative to the peacock body image
    const double PEACOCK_WITHIN_CENTER_Y_LOC = 350.0/820; //rotation point y for feathers relative to the peacock body image
    const double PEACOCK_FEATHER_LENGTH = 0.12;
    const int    FEATHER_COUNT = 5; //MIN_PARAM_FOR_FEATHER represents the number of feathers on each side. Total feathers would be 2n+1 from this (i.e. if MIN_PARAM_FOR_FEATHER were 5, there would be 11 total feathers, 5 left, 1 central, and 5 right)
    const double MIN_PARAM_FOR_FEATHER = -0.2;
    const double MAX_PARAM_FOR_FEATHER = +0.5;
    const double MIN_FEATHER_SPREAD_ANGLE =   0; //degrees, is the min that spread angle is thresholded at and is the spread that corresponds to MIN_PARAM_FOR_FEATHER
    const double MAX_FEATHER_SPREAD_ANGLE = 210; //degrees, is the max that spread angle is thresholded at and is the spread that corresponds to MAX_PARAM_FOR_FEATHER
    const double DEFAULT_FEATHER_SPREAD_ANGLE = 105; //degrees, is the value when no hand is present, etc
    
    cv::Mat peacockGameScenery;
    cv::Mat peacockBody;
    cv::Mat peacockFeather;
    
    cv::Mat peacockGameSceneryOrig;
    cv::Mat peacockBodyOrig;
    cv::Mat peacockFeatherOrig;
    
    bool peacockImagesInitialized = false;
    
    
#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setPeacockGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, peacockGameSceneryOrig, cv::COLOR_BGRA2RGBA);
        peacockImagesInitialized = false;
    }
    void
    Java_com_motracktherapy_motrack_CVActivity_setPeacockGamePeacockBody(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, peacockBodyOrig, cv::COLOR_BGRA2RGBA);
        peacockImagesInitialized = false;
    }
    void
    Java_com_motracktherapy_motrack_CVActivity_setPeacockGameFeather(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, peacockFeatherOrig, cv::COLOR_BGRA2RGBA);
        peacockImagesInitialized = false;
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setPeacockGameScenery(cv::Mat picture) {
        if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
        peacockGameSceneryOrig = picture;
        peacockImagesInitialized = false;
    }
    void setPeacockGamePeacockBody(cv::Mat picture) {
        peacockBodyOrig = picture;
        peacockImagesInitialized = false;
    }
    void setPeacockGameFeather(cv::Mat picture) {
        peacockFeatherOrig = picture;
        peacockImagesInitialized = false;
    }
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */
    
    
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_peacock_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        if (!peacockImagesInitialized) {
            if (whichHand == Hand::LEFT) {
                cv::flip(peacockGameSceneryOrig, peacockGameSceneryOrig, +1);
                peacockGameScenery.data = NULL;
                cv::flip(peacockBodyOrig, peacockBodyOrig, +1);
                peacockBody.data = NULL;
                cv::flip(peacockFeatherOrig, peacockFeatherOrig, +1);
                peacockFeather.data = NULL;
            }
            peacockImagesInitialized = true;
        }
        
        int peacock_body_center_x = 0;
        int peacock_body_center_y = 0;
        Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(0, textWriteRectangle.height, frame_with_mask.cols, frame_with_mask.rows-textWriteRectangle.height)) : frame_with_mask;
        int peacock_body_ht = PEACOCK_BODY_HEIGHT * frame_with_mask_draw.rows;

        
        double param = analyze_fabduction(fgmask, frame_with_mask, dir, ratio);
        
        
        if (peacockGameSceneryOrig.data != NULL) {
            cv::resize(peacockGameSceneryOrig, peacockGameScenery, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, INTER_LINEAR);
            
            
            //Draw peacock feathers
            if (peacockFeatherOrig.data != NULL) {
                //peacockFeather must be square but peacockFeatherOrig doesn't have to be
                if ( peacockFeather.data == NULL || peacockFeather.cols != peacockFeather.rows || abs(peacockFeather.cols*1.0/frame_with_mask_draw.cols - PEACOCK_FEATHER_LENGTH) > 0.01 ) {
                    double scale = PEACOCK_FEATHER_LENGTH * frame_with_mask_draw.cols*1.0/peacockFeatherOrig.cols;
                    cv::resize(peacockFeatherOrig, peacockFeather, cv::Size(), scale, scale );
                    
                    
                    //make bottom half of the peacock area so that center of rotation is the middle
                    cv::copyMakeBorder(peacockFeather, peacockFeather, 0, peacockFeather.rows, 0, 0, BORDER_CONSTANT, Scalar(0,0,0,0)); //transparent
                    
                    //make image square
                    int top    = max(0,peacockFeather.cols-peacockFeather.rows)/2;
                    int bottom = max(0,peacockFeather.cols-peacockFeather.rows)-top;
                    int left   = max(0,peacockFeather.rows-peacockFeather.cols)/2;
                    int right  = max(0,peacockFeather.rows-peacockFeather.cols)-left;
                    cv::copyMakeBorder(peacockFeather, peacockFeather, top, bottom, left, right, BORDER_CONSTANT, Scalar(0,0,0,0)); //transparent
                    
                } else {
                    
                    
                }
                
                //feather x center will be center of peacock body x and feather y center will be center of peacock body y
                int start_x_feather = frame_with_mask_draw.cols*PEACOCK_BODY_CENTER_X_LOC - peacockFeather.cols/2;
                int start_y_feather = frame_with_mask_draw.rows*PEACOCK_BODY_CENTER_Y_LOC - peacockFeather.rows/2;
                
                //spread_angle is in degrees and represents the spread amount (i.e. angle between leftmost and rightmost feather)
                double spread_angle = MIN_FEATHER_SPREAD_ANGLE + (param - MIN_PARAM_FOR_FEATHER)*(MAX_FEATHER_SPREAD_ANGLE-MIN_FEATHER_SPREAD_ANGLE)/(MAX_PARAM_FOR_FEATHER-MIN_PARAM_FOR_FEATHER); //make spread_angle equal to the min spread angle if param is the min param and equal to the max spread angle if param is the max param (and linearly between)
                if (isParamNoExerciseDone(param)) spread_angle=DEFAULT_FEATHER_SPREAD_ANGLE;
                
                if (displayExtraInfoAmount == 0) { //do only if normal (not testing) mode
                    spread_angle = max( MIN_FEATHER_SPREAD_ANGLE, min(spread_angle,MAX_FEATHER_SPREAD_ANGLE) ); //threshold spread_angle so that it is between the min and max feather spread angle

                }
                
                
                /*for (int i = 0; i < FEATHER_COUNT*2+1; i++) {
                    double degrees = -spread_angle/2 + i*spread_angle/(FEATHER_COUNT*2); //first feather is at -spread/2 angle and last feather is at +spread/2 angle
                    Mat peacockFeatherRotated;
                    Point2f rotPt (peacockFeather.cols / 2.,  peacockFeather.rows/2);
                    Mat rotMatBack = getRotationMatrix2D(rotPt,  -degrees, 1.0); //positive angle is counterclockwise, angle is in degrees
                    warpAffine(peacockFeather,  peacockFeatherRotated,  rotMatBack,  Size(peacockFeather.cols,  peacockFeather.rows));
                    drawImageOptimized(peacockGameScenery, peacockFeatherRotated, start_x_feather, start_y_feather);
                }*/
                
                /*for (int i = 0; i < FEATHER_COUNT*2+1; i++) {
                    double degrees = -spread_angle/2 + i*spread_angle/(FEATHER_COUNT*2); //first feather is at -spread/2 angle and last feather is at +spread/2 angle
                    if (abs(degrees) < 0.00001 && false) drawImageOptimized(peacockGameScenery, peacockFeather, start_x_feather, start_y_feather);
                    else {
                        Mat peacockFeatherRotated;
                        Point2f rotPt (peacockFeather.cols / 2.,  peacockFeather.rows/2);
                        Mat rotMatBack = getRotationMatrix2D(rotPt,  -degrees, 1.0); //positive angle is counterclockwise, angle is in degrees
                        warpAffine(peacockFeather,  peacockFeatherRotated,  rotMatBack,  Size(peacockFeather.cols,  peacockFeather.rows));
                        drawImageOptimized(peacockGameScenery, peacockFeatherRotated, start_x_feather, start_y_feather);
                    }
                }*/
                
                
                //draw side ones
                for (int i = FEATHER_COUNT; i > 0; i--) { //NOTE the index being 1-index not 0-indexed to be more intuitive i.e. i is then the feather number
                    double degrees = i*1.0/FEATHER_COUNT*spread_angle/2; //if spread angle were 90 and FEATHER_COUNT were 5 then degrees would be 18 then 36 then 54 then 72 then 90
                    Mat peacockFeatherRotated;
                    Point2f rotPt (peacockFeather.cols / 2.,  peacockFeather.rows/2);
                    //Be careful! I tried using the scale parameter to make it more realistic with some feathers smaller than others, but that apparently also changes the perceived angle? (discovered empirically)
                    Mat rotMatBack = getRotationMatrix2D(rotPt,  -degrees, 1 ); //positive angle is counterclockwise, angle is in degrees
                    warpAffine(peacockFeather,  peacockFeatherRotated,  rotMatBack,  Size(peacockFeather.cols,  peacockFeather.rows)); //NOTE: This is the most time intensive step so anything we can do to minimize the number of times it is called (i.e. skipping calling it for the central 0 degree feather and not calling it once per each side, thereby making n called instead of 2n+1 calls where n=FEATHER_COUNT) will make it faster. At 2n+1, it wouldn't run well for a feather count of 5 on my iPhone 6 on Fri Mar 22, 2019 (~33 psec for the do_peacock_game method)
                    drawImageOptimized(peacockGameScenery, peacockFeatherRotated, start_x_feather, start_y_feather);
                    
                    cv::flip(peacockFeatherRotated, peacockFeatherRotated, +1); //flip left/right to get the opposite angle feather
                    drawImageOptimized(peacockGameScenery, peacockFeatherRotated, start_x_feather, start_y_feather);
                }
                
                //draw central (0 degree angle) last, so it is on top
                drawImageOptimized(peacockGameScenery, peacockFeather, start_x_feather, start_y_feather);
            }
            
            
            //Draw peacock body
            if (peacockBodyOrig.data != NULL) {
                if ( peacockBody.data == NULL || abs(peacockBody.rows*1.0/frame_with_mask_draw.rows - PEACOCK_BODY_HEIGHT) > 0.01 ) {
                    double scale = PEACOCK_BODY_HEIGHT * frame_with_mask_draw.rows/peacockBodyOrig.rows;
                    cv::resize(peacockBodyOrig, peacockBody, cv::Size(), scale, scale );
                } else {
                    peacock_body_ht = peacockBody.rows;
                }
                
                int start_x;
                if (whichHand == Hand::LEFT) {
                    start_x = frame_with_mask_draw.cols*PEACOCK_BODY_CENTER_X_LOC - peacockBody.cols*(1-PEACOCK_WITHIN_CENTER_X_LOC); //convert from from center_x_loc position to left side
                } else {
                    start_x = frame_with_mask_draw.cols*PEACOCK_BODY_CENTER_X_LOC - peacockBody.cols*PEACOCK_WITHIN_CENTER_X_LOC; //convert from from center_x_loc position to left side
                }
                
                int start_y = frame_with_mask_draw.rows*PEACOCK_BODY_CENTER_Y_LOC - peacockBody.rows*PEACOCK_WITHIN_CENTER_X_LOC;   //convert from from center_x_loc position to top side
                
                drawImageOptimized(peacockGameScenery, peacockBody, start_x, start_y);
                
                peacock_body_center_x = start_x + peacockBody.cols* ( whichHand == Hand::LEFT ?  1-PEACOCK_WITHIN_CENTER_X_LOC : PEACOCK_WITHIN_CENTER_X_LOC);
                peacock_body_center_y = start_y + peacockBody.rows*PEACOCK_WITHIN_CENTER_Y_LOC;
            }
            
            
            // for text that will be written on top/side
            if (shouldWriteOnIm) {
                //Don't need to draw the rectangle since copyMakeBorder already does the same effect
                cv::copyMakeBorder(peacockGameScenery, peacockGameScenery, textWriteRectangle.height, 0, 0, 0, BORDER_CONSTANT, textWriteRectangleBackground);
                //cv::rectangle(peacockGameScenery, textWriteRectangle, textWriteRectangleBackground, -1, 8);

                cv::line(peacockGameScenery, cv::Point(0, textWriteRectangle.height), cv::Point(peacockGameScenery.cols, textWriteRectangle.height), CLR_BLACK_4UC, 5);

            }
            
            
            Mat bgmask;
            bitwise_not(fgmask, bgmask);
            cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
            bitwise_and(peacockGameScenery, peacockGameScenery, frame_with_mask, bgmask);
            
        }
        
        
        return param;
        
    }
    
    
#ifdef EXTERN_C
}
#endif
