//
//  topposition_alligator.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/30/19.
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

#include "topposition_alligator.h"
#include "topposition_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    
    const double ALLIGATOR_BODY_WIDTH = 0.4;
    const double ALLIGATOR_UPPER_JAW_ROTATION_X_WITHIN_BODY = 1450.0/2480;
    const double ALLIGATOR_UPPER_JAW_ROTATION_Y_WITHIN_BODY = (2690.0-2147)/3508;
    const double ALLIGATOR_LOWER_JAW_ROTATION_X_WITHIN_BODY = 797.0/2480;
    const double ALLIGATOR_LOWER_JAW_ROTATION_Y_WITHIN_BODY = (2700.0-2147)/3508;
    const double ALLIGATOR_MOUTH_STARTING_IMAGE_ROTATION = 35; //the amount the mouth images are already rotated when added as assets to the app
    
    const double ALLIGATOR_UPPER_JAW_ROTATION_X_WITHIN_UPPER_MOUTH = 108.0/266;
    const double ALLIGATOR_UPPER_JAW_ROTATION_Y_WITHIN_UPPER_MOUTH = 566.0/566;
    const double ALLIGATOR_LOWER_JAW_ROTATION_X_WITHIN_LOWER_MOUTH = 122.0/182;
    const double ALLIGATOR_LOWER_JAW_ROTATION_Y_WITHIN_LOWER_MOUTH = 500.0/500;

    
    cv::Mat alligatorGameScenery;
    cv::Mat alligatorBody;
    cv::Mat alligatorUpperMouth;
    cv::Mat alligatorLowerMouth;
    
    cv::Mat alligatorGameSceneryOrig;
    cv::Mat alligatorBodyOrig;
    cv::Mat alligatorUpperMouthOrig;
    cv::Mat alligatorLowerMouthOrig;
    
    bool alligatorImagesInitialized = false;


#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
    extern "C" {
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameScenery(JNIEnv *env, jobject, long addr) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, alligatorGameSceneryOrig, cv::COLOR_BGRA2RGBA);
        alligatorImagesInitialized = false;
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameAlligatorBody(JNIEnv *env, jobject, long addr) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, alligatorBodyOrig, cv::COLOR_BGRA2RGBA);
        alligatorImagesInitialized = false;
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameAlligatorUpperMouth(JNIEnv *env, jobject, long addr) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, alligatorUpperMouthOrig, cv::COLOR_BGRA2RGBA);
        alligatorImagesInitialized = false;
    }
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameAlligatorLowerMouth(JNIEnv *env, jobject, long addr) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, alligatorLowerMouthOrig, cv::COLOR_BGRA2RGBA);
        alligatorImagesInitialized = false;
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setAlligatorGameScenery(cv::Mat picture) {
        if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
        alligatorGameSceneryOrig = picture;
        alligatorImagesInitialized = false;
    }
    
    void setAlligatorGameAlligatorBody(cv::Mat picture) {
        alligatorBodyOrig = picture;
        alligatorImagesInitialized = false;
    }
    
    void setAlligatorUpperMouth(cv::Mat picture) {
        alligatorUpperMouthOrig = picture;
        alligatorImagesInitialized = false;
    }
    void setAlligatorLowerMouth(cv::Mat picture) {
        alligatorLowerMouthOrig = picture;
        alligatorImagesInitialized = false;
    }
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */
    
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_alligator_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        if (!alligatorImagesInitialized) {
            if (alligatorGameSceneryOrig.data == NULL) {
                //light green
                alligatorGameSceneryOrig = cv::Mat(25,25, CV_8UC4, cv::Scalar(167, 200, 129, 255));
                //alligatorGameSceneryOrig.setTo( cv::Scalar(167, 200, 129, 255) );
            }
            
            if (whichHand == Hand::LEFT) {
                cv::flip(alligatorGameSceneryOrig, alligatorGameSceneryOrig, +1);
                alligatorGameScenery.data = NULL;
                cv::flip(alligatorBodyOrig, alligatorBodyOrig, +1);
                alligatorBody.data = NULL;
                //Until I figure out the math for flipping each one and implement it, I will leave left and right identical for now
                //cv::flip(alligatorUpperMouthOrig, alligatorUpperMouthOrig, +1);
                alligatorUpperMouth.data = NULL;
                //cv::flip(alligatorLowerMouthOrig, alligatorLowerMouthOrig, +1);
                alligatorLowerMouth.data = NULL;

            }
            alligatorImagesInitialized = true;
        }
        

        
        Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(0, textWriteRectangle.height, frame_with_mask.cols, frame_with_mask.rows-textWriteRectangle.height)) : frame_with_mask;
        
        
        //param is returned in units of degrees
        double param = analyze_thumb_opposition(fgmask, frame_with_mask, dir, ratio);
        
        
        if (alligatorGameSceneryOrig.data != NULL) {
            cv::resize(alligatorGameSceneryOrig, alligatorGameScenery, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, INTER_LINEAR);
            
            
            int alligator_body_wd = ALLIGATOR_BODY_WIDTH * frame_with_mask_draw.cols;
            if (alligatorBodyOrig.data != NULL && alligatorUpperMouthOrig.data != NULL && alligatorLowerMouthOrig.data != NULL) {
                if ( alligatorBody.data == NULL || abs(alligatorBody.cols*1.0/frame_with_mask_draw.cols - ALLIGATOR_BODY_WIDTH) > 0.01 ) {
                    double scale = ALLIGATOR_BODY_WIDTH * frame_with_mask_draw.cols/alligatorBodyOrig.cols;
                    cv::resize(alligatorBodyOrig, alligatorBody, cv::Size(), scale, scale );
                    cv::resize(alligatorUpperMouthOrig, alligatorUpperMouth, cv::Size(), scale, scale );
                    cv::resize(alligatorLowerMouthOrig, alligatorLowerMouth, cv::Size(), scale, scale );
                    
                } else {
                    alligator_body_wd = alligatorBody.cols;
                }
                
                bool noExerciseDone = isParamNoExerciseDone(param);
                Mat alligatorCombined(alligatorGameScenery.size(), CV_8UC4, Scalar(0,0,0,0));

                //Draw alligator body
                int start_x_body = (alligatorGameScenery.cols-alligatorBody.cols)/2; //make center horizontally
                int start_y_body = alligatorGameScenery.rows/2 - alligatorBody.rows; //make bottom be in the center
                drawImageOptimized(alligatorCombined, alligatorBody, start_x_body, start_y_body);
                
                //Create image of the alligator mouth, upper part
                Mat alligatorUpperMouthPadded;
                float mouth_upper_diagonal = sqrt(alligatorUpperMouth.rows*alligatorUpperMouth.rows + alligatorUpperMouth.cols*alligatorUpperMouth.cols);
                int padding_upper_top = (mouth_upper_diagonal-alligatorUpperMouth.rows)*ALLIGATOR_UPPER_JAW_ROTATION_Y_WITHIN_UPPER_MOUTH;
                int padding_upper_bottom = mouth_upper_diagonal-alligatorUpperMouth.rows-padding_upper_top;
                int padding_upper_left = (mouth_upper_diagonal-alligatorUpperMouth.cols)*ALLIGATOR_UPPER_JAW_ROTATION_X_WITHIN_UPPER_MOUTH;
                int padding_upper_right = mouth_upper_diagonal-alligatorUpperMouth.cols-padding_upper_left;
                cv::copyMakeBorder(alligatorUpperMouth, alligatorUpperMouthPadded, padding_upper_top, padding_upper_bottom, padding_upper_left, padding_upper_right, BORDER_CONSTANT, cv::Scalar(0,0,0,0));
                Mat alligatorUpperMouthRotated;
                double degrees_upper = noExerciseDone ? 0 : -(param-ALLIGATOR_MOUTH_STARTING_IMAGE_ROTATION)/2*0.5;
                Point2f rotPtUpper(padding_upper_left+ALLIGATOR_UPPER_JAW_ROTATION_X_WITHIN_UPPER_MOUTH*alligatorUpperMouth.cols,  padding_upper_top+ALLIGATOR_UPPER_JAW_ROTATION_Y_WITHIN_UPPER_MOUTH*alligatorUpperMouth.rows);
                Mat rotMatUpper = getRotationMatrix2D(rotPtUpper,  degrees_upper, 1.0); //positive angle is counterclockwise, angle is in degrees
                warpAffine(alligatorUpperMouthPadded,  alligatorUpperMouthRotated,  rotMatUpper,  Size(alligatorUpperMouthPadded.cols,  alligatorUpperMouthPadded.rows));

                //Create image of the alligator mouth, lower part
                Mat alligatorLowerMouthPadded;
                float mouth_lower_diagonal = sqrt(alligatorLowerMouth.rows*alligatorLowerMouth.rows + alligatorLowerMouth.cols*alligatorLowerMouth.cols);
                int padding_lower_top = (mouth_upper_diagonal-alligatorLowerMouth.rows)*ALLIGATOR_LOWER_JAW_ROTATION_Y_WITHIN_LOWER_MOUTH;
                int padding_lower_bottom = mouth_upper_diagonal-alligatorLowerMouth.rows-padding_lower_top;
                int padding_lower_left = (mouth_upper_diagonal-alligatorLowerMouth.cols)*ALLIGATOR_LOWER_JAW_ROTATION_X_WITHIN_LOWER_MOUTH;
                int padding_lower_right = mouth_upper_diagonal-alligatorLowerMouth.cols-padding_upper_left;
                cv::copyMakeBorder(alligatorLowerMouth, alligatorLowerMouthPadded, padding_lower_top, padding_lower_bottom, padding_lower_left, padding_lower_right, BORDER_CONSTANT, cv::Scalar(0,0,0,0));
                Mat alligatorLowerMouthRotated;
                double degrees_lower = isParamNoExerciseDone(param) ? 0 : +(param-ALLIGATOR_MOUTH_STARTING_IMAGE_ROTATION)/2*0.5;
                Point2f rotPtLower(padding_lower_left+ALLIGATOR_LOWER_JAW_ROTATION_X_WITHIN_LOWER_MOUTH*alligatorLowerMouth.cols,  padding_lower_top+ALLIGATOR_LOWER_JAW_ROTATION_Y_WITHIN_LOWER_MOUTH*alligatorLowerMouth.rows);
                Mat rotMatLower = getRotationMatrix2D(rotPtLower,  degrees_lower, 1.0); //positive angle is counterclockwise, angle is in degrees
                warpAffine(alligatorLowerMouthPadded,  alligatorLowerMouthRotated,  rotMatLower,  Size(alligatorLowerMouthPadded.cols,  alligatorLowerMouthPadded.rows));
                

                //Draw alligator mouth
                //for some reason, it doesn't line up properly, but it works okay if I subtract approximately 0.1
                int start_x_upper_mouth = start_x_body + ALLIGATOR_UPPER_JAW_ROTATION_X_WITHIN_BODY*alligatorBody.cols - ALLIGATOR_UPPER_JAW_ROTATION_X_WITHIN_UPPER_MOUTH*alligatorUpperMouth.cols - padding_upper_left;
                int start_y_upper_mouth = start_y_body + ALLIGATOR_UPPER_JAW_ROTATION_Y_WITHIN_BODY*alligatorBody.rows - (ALLIGATOR_UPPER_JAW_ROTATION_Y_WITHIN_UPPER_MOUTH-0.1)*alligatorUpperMouth.rows - padding_upper_top;
                drawImageOptimized(alligatorCombined, alligatorUpperMouthRotated, start_x_upper_mouth, start_y_upper_mouth);

                int start_x_lower_mouth = start_x_body + ALLIGATOR_LOWER_JAW_ROTATION_X_WITHIN_BODY*alligatorBody.cols - ALLIGATOR_LOWER_JAW_ROTATION_X_WITHIN_LOWER_MOUTH*alligatorLowerMouth.cols - padding_lower_left;
                int start_y_lower_mouth = start_y_body + ALLIGATOR_LOWER_JAW_ROTATION_Y_WITHIN_BODY*alligatorBody.rows - (ALLIGATOR_LOWER_JAW_ROTATION_Y_WITHIN_LOWER_MOUTH-0.1)*alligatorLowerMouth.rows - padding_lower_top;
                drawImageOptimized(alligatorCombined, alligatorLowerMouthRotated, start_x_lower_mouth, start_y_lower_mouth);
                
                if (noExerciseDone) {
                    // forming an array of matrices is a quite efficient operation,
                    // because the matrix data is not copied, only the headers
                    Mat out[] = { alligatorCombined };
                    // takes the green value and sets it to the blue and the red, and keeps the alpha
                    int from_to[] = { 1,0,  1,1,  1,2,  3,3 };
                    mixChannels( &alligatorCombined, 1, out, 1, from_to, 4 );
                }
                drawImageOptimized(alligatorGameScenery, alligatorCombined, 0, 0);


            }


            
            
            // for text that will be written on top/side
            if (shouldWriteOnIm) {
                //Don't need to draw the rectangle since copyMakeBorder already does the same effect
                cv::copyMakeBorder(alligatorGameScenery, alligatorGameScenery, textWriteRectangle.height, 0, 0, 0, BORDER_CONSTANT, textWriteRectangleBackground);
                
                cv::line(alligatorGameScenery, cv::Point(0, textWriteRectangle.height), cv::Point(alligatorGameScenery.cols, textWriteRectangle.height), CLR_BLACK_4UC, 5);
                
            }
            
            
            Mat bgmask;
            bitwise_not(fgmask, bgmask);
            cv::resize(bgmask, bgmask, frame_with_mask.size(), 0, 0, INTER_NEAREST );
            bitwise_and(alligatorGameScenery, alligatorGameScenery, frame_with_mask, bgmask);
            
        }
        
        
        return param;
        
    }
    
    
#ifdef EXTERN_C
}
#endif
