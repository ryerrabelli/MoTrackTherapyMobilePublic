//
//  pronation_dial.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/19/19.
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

#include "pronation_dial.h"
#include "pronation_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif

    
    cv::Mat ovenBackground;
    cv::Mat dialFrame;
    cv::Mat dialPointer;

    cv::Mat ovenBackgroundOrig;
    cv::Mat dialFrameOrig;
    cv::Mat dialPointerOrig;

    bool dialImagesInitialized = false;


#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setDialFrame(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, dialFrameOrig, cv::COLOR_BGRA2RGBA);
        dialImagesInitialized = false;
    }
    void
    Java_com_motracktherapy_motrack_CVActivity_setOvenBackground(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, ovenBackgroundOrig, cv::COLOR_BGRA2RGBA);
        dialImagesInitialized = false;
    }
    void
    Java_com_motracktherapy_motrack_CVActivity_setDialPointer(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, dialPointerOrig, cv::COLOR_BGRA2RGBA);
        dialImagesInitialized = false;
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setDialFrame(cv::Mat picture) {
        dialFrameOrig = picture;
        //double scale = 250*250 * 1.0/( dialFrame.rows*dialFrame.cols );
        //cv::resize(dialFrameOrig, dialFrameOrig, cv::Size(), scale, scale, cv::INTER_NEAREST);
        dialImagesInitialized = false;
    }
    void setOvenBackground(cv::Mat picture) {
        ovenBackgroundOrig = picture;
        dialImagesInitialized = false;
    }
    void setDialPointer(cv::Mat picture) {
        dialPointerOrig = picture;
        dialImagesInitialized = false;
    }
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */
    
    
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_dial_game(Mat fgmask, Mat &frame_with_mask, double ratio) {
        Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(0, textWriteRectangle.height, frame_with_mask.cols, frame_with_mask.rows-textWriteRectangle.height)) : frame_with_mask;

        if (!dialImagesInitialized) {
            //double dialFrameDiameter;
            //cv::resize(dialFrameOrig, dialFrame, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, INTER_LINEAR);
            double scale = frame_with_mask_draw.cols*0.5/dialFrameOrig.cols;
            cv::resize(dialFrameOrig, dialFrame, cv::Size(), scale, scale, INTER_LINEAR);
            cv::resize(dialPointerOrig, dialPointer, cv::Size(), scale/2, scale/2, INTER_LINEAR);
            ovenBackground = ovenBackgroundOrig.clone();
            dialImagesInitialized = true;
        }


        //NOTE: The background needs to be before the calcRotation function because it needs to be done before writeText is done to print out the "P %.2f" etc buffer string. Otherwise, only the part of the string that is located where the hand is will show.
        Mat backgroundResized;
        cv::resize(ovenBackground, backgroundResized, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, cv::INTER_NEAREST);
        double dialFrameLeftEdge = (backgroundResized.cols-dialFrame.cols)/2; //measured from left edge of screen. It's a bit off center (center would be frame_with_mask.cols/2-wiperBottomResized.cols/2) so that it doesn't overlap with the road lines.
        double dialFrameTopEdge = 0/2; //measured from top edge of screen
        //copyWithPixelTransparencyUsingAlpha(backgroundResized, dialFrame, dialFrameLeftEdge, dialFrameTopEdge, dialFrame.cols, dialFrame.rows, 250);
        drawImageOptimized(backgroundResized, dialFrame, dialFrameLeftEdge, dialFrameTopEdge);


        // for text that will be written on top
        if (shouldWriteOnIm) {
            //Don't need to draw the rectangle since copyMakeBorder already does the same effect
            cv::copyMakeBorder(backgroundResized, backgroundResized, textWriteRectangle.height, 0, 0, 0, BORDER_CONSTANT, textWriteRectangleBackground);
            //cv::rectangle(backgroundResized, textWriteRectangle, textWriteRectangleBackground, -1, 8);

            cv::line(backgroundResized, cv::Point(0, textWriteRectangle.height), cv::Point(backgroundResized.cols, textWriteRectangle.height), CLR_BLACK_4UC, 5);

        }
        
        Mat bgmask;
        bitwise_not(fgmask,bgmask);
        cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
        bitwise_and(backgroundResized, backgroundResized, frame_with_mask, bgmask);
        
        
        double param = analyze_pronation(fgmask, frame_with_mask, ratio);
        

        Mat dialPointerResized = dialPointer.clone();
        /*
        double dialPointerMultiplier = 0.5;
        Mat dialPointerResized;
        cv::resize(dialPointer, dialPointerResized,
                   cv::Size(dialPointer.cols*dialPointerMultiplier, dialPointer.rows*dialPointerMultiplier),
                   0, 0, cv::INTER_NEAREST);
        */
        
        Point2f pt(dialPointerResized.cols/2.0, dialPointerResized.rows/2.0);
        double degrees = isParamNoExerciseDone(param) ? 0 : param*180.0; //in degrees, not radians. Positive values mean counterclockwise.
        Mat rotMat = getRotationMatrix2D(pt, degrees, 1.0);
        warpAffine(dialPointerResized, dialPointerResized, rotMat, Size(dialPointerResized.cols, dialPointerResized.rows));
        drawImageOptimized(frame_with_mask_draw, dialPointerResized, (frame_with_mask_draw.cols-dialPointerResized.cols)/2, dialFrameTopEdge+(dialFrame.rows-dialPointerResized.rows)/2);
        return param;
        
    }
    
    
#ifdef EXTERN_C
}
#endif
