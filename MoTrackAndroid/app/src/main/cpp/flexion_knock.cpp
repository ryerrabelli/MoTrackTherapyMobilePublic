//
// Created by Rahul Yerrabelli on 12/18/18.
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

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    //Below are the imports for the iOS version

#endif


// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cmath>

#include "flexion_knock.h"
#include "bilinear_approx.h"



using namespace cv;
using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif

//cv::Mat knockGameScenery(100, 100, CV_8UC4); //necessary to create tester solid image using .setTo( function
cv::Mat knockGameScenery;

#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
extern "C" {

void
Java_com_motracktherapy_motrack_CVActivity_setKnockGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    bool includes_alpha = (bool) jincludes_alpha;
    cv::Mat* Address = (cv::Mat*)addr;
    cv::cvtColor(*Address, knockGameScenery, cv::COLOR_BGRA2RGBA);
    if (whichHand == Hand::LEFT) { // flip for left
        cv::flip(knockGameScenery, knockGameScenery, 1);
    }
}

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * -------------------------------------------------------------------------------------------------
 *                                     iOS INTERFACE FUNCTIONS 
 * -------------------------------------------------------------------------------------------------
 */

void
setKnockGameScenery(cv::Mat knockImageMat) {
    knockGameScenery = knockImageMat;
    if (whichHand == Hand::LEFT) { // flip for left
        cv::flip(knockGameScenery, knockGameScenery, 1);
    }
}


#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */


int do_knock_game(Mat& fgmask, Mat &frame_with_mask, int currentMot, double ratio) {
    Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(textWriteRectangle.width, 0, frame_with_mask.cols-textWriteRectangle.width, frame_with_mask.rows)) : frame_with_mask;

    Mat sceneryResized;
    cv::resize(knockGameScenery, sceneryResized, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, INTER_LINEAR);

    // for text that will be written on top/side
    if (shouldWriteOnIm) {
        //Don't need to draw the rectangle since copyMakeBorder already does the same effect
        cv::copyMakeBorder(sceneryResized, sceneryResized, 0, 0, textWriteRectangle.width, 0, BORDER_CONSTANT, textWriteRectangleBackground);
        //cv::rectangle(fieldScenery, textWriteRectangle, textWriteRectangleBackground, -1, 8);

        cv::line(sceneryResized, cv::Point(textWriteRectangle.width,0), cv::Point(textWriteRectangle.width, sceneryResized.rows), CLR_BLACK_4UC, 5);

    }

    Mat bgmask;
    bitwise_not(fgmask,bgmask);
    cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
    if (colorMode == 2) { frame_with_mask.setTo(CLR_DISABILITY_RED_4UC, bgmask); } //didn't actually test, but I presume this is faster than bitwise_and so let's do this
    else bitwise_and(sceneryResized, sceneryResized, frame_with_mask, bgmask);
    return bilinear_approx_analysis(fgmask, frame_with_mask, currentMot, ratio, true);
}

#ifdef EXTERN_C
}
#endif
