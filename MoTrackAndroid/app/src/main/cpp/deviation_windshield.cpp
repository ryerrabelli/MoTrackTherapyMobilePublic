//
// Created by Rahul Yerrabelli on 11/30/18.
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

#include "deviation_windshield.h"
#include "bilinear_approx.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif

cv::Mat carFrame;

cv::Mat windshieldWiperBottom;
cv::Mat windshieldWiperTop;
cv::Mat steeringWheel;
cv::Mat scenery;
    
bool windshieldImagesInitialized = false;


#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
extern "C" {

void
Java_com_motracktherapy_motrack_CVActivity_setCarFrame(JNIEnv *env, jobject, long addrCarFrame, jboolean jincludes_alpha) {
    bool includes_alpha = (bool) jincludes_alpha;
    if (addrCarFrame != -1) {
        cv::Mat *carFrameAddress = (cv::Mat *) addrCarFrame;
        //below line would set it directly. Can't do this because we want to convert the color
        //carFrame = *carFrameAddress;
        cv::cvtColor(*carFrameAddress, carFrame, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) cv::flip(carFrame, carFrame, +1);
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldGameScenery(JNIEnv *env, jobject, long addrScenery, jboolean jincludes_alpha) {
    if (addrScenery != -1) {
        cv::Mat *sceneryAddress = (cv::Mat *) addrScenery;
        //below line would set it directly. Can't do this because we want to convert the color
        //carFrame = *carFrameAddress;
        cv::cvtColor(*sceneryAddress, scenery, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) cv::flip(scenery, scenery, +1);
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldWiperBottom(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        cv::Mat *Address = (cv::Mat *) addr;
        cv::cvtColor(*Address, windshieldWiperBottom, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) cv::flip(windshieldWiperBottom, windshieldWiperBottom, +1);
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldWiperTop(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        cv::Mat *Address = (cv::Mat *) addr;
        cv::cvtColor(*Address, windshieldWiperTop, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) cv::flip(windshieldWiperTop, windshieldWiperTop, +1);
    }
}


void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldGameSteeringWheel(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        cv::Mat *Address = (cv::Mat *) addr;
        cv::cvtColor(*Address, steeringWheel, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) cv::flip(steeringWheel, steeringWheel, +1);
    }
}

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
void setCarFrame(cv::Mat picture) {
    if (whichHand == Hand::LEFT) cv::flip(picture, picture, +1);
    carFrame = picture;
}
void setWindshieldGameScenery(cv::Mat picture) {
    if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
    if (whichHand == Hand::LEFT) cv::flip(picture, picture, +1);
    scenery = picture;
}
void setWindshieldWiperBottom(cv::Mat picture) {
    if (whichHand == Hand::LEFT) cv::flip(picture, picture, +1);
    windshieldWiperBottom = picture;
}
void setWindshieldWiperTop(cv::Mat picture) {
    if (whichHand == Hand::LEFT) cv::flip(picture, picture, +1);
    windshieldWiperTop = picture;
}

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */



// main method that does game graphics and bilinear approximation, returning angle
int do_windshield_game(Mat& fgmask, Mat &frame_with_mask, int dir, double ratio) {
    if (scenery.data != NULL) {
        Mat sceneryResized;
        cv::resize(scenery, sceneryResized, cv::Size(frame_with_mask.cols, frame_with_mask.rows),
                   0,
                   0, INTER_LINEAR);
        Mat bgmask;
        bitwise_not(fgmask, bgmask);
        cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );

        if (colorMode == 2) { frame_with_mask.setTo(CLR_DISABILITY_RED_4UC, bgmask); } //didn't actually test, but I presume this is faster than bitwise_and so let's do this
        else bitwise_and(sceneryResized, sceneryResized, frame_with_mask, bgmask);
    }
    if (carFrame.data != NULL) {
        Mat carFrameResized;
        cv::resize(carFrame, carFrameResized, cv::Size(frame_with_mask.cols, frame_with_mask.rows),
                   0, 0, cv::INTER_NEAREST);
        
        /*copyWithPixelTransparencyUsingAlpha(frame_with_mask, carFrameResized, 0, 0,
                                            carFrameResized.cols, carFrameResized.rows,
                                            250);*/
        drawImageOptimized(frame_with_mask, carFrameResized, 0,0);
    }

    // for text that will be written on top
    if (shouldWriteOnIm) {
        cv::rectangle(frame_with_mask, textWriteRectangle, textWriteRectangleBackground, -1, 8);
        /*if (fgmask.cols > fgmask.rows) { //landscape
            cv::line(sceneryResized,cv::Point(textWriteRectangle.width,0),cv::Point(textWriteRectangle.width,sceneryResized.rows),cv::Scalar(0,0,0,255),5);
        } else { // portrait
            cv::line(sceneryResized,cv::Point(0,textWriteRectangle.height),cv::Point(sceneryResized.cols,textWriteRectangle.height),cv::Scalar(0,0,0,255),5);

        }*/
        cv::line(frame_with_mask,cv::Point(textWriteRectangle.width,0),cv::Point(textWriteRectangle.width,frame_with_mask.rows),cv::Scalar(0,0,0,255),5);
    }
    chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
    
    int angle = bilinear_approx_analysis(fgmask, frame_with_mask, dir, ratio, true);
    
    chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    cout << time_span.count()/1000.0 << "ms" << endl; //*/
    

    //double wiperMultiplier = .15;
    double wiperMultiplier = 0.15 * frame_with_mask.cols/480;
    //double wiperMultiplier = 0.4 * frame_with_mask.cols *  1/windshieldWiperBottom.cols;
    if (windshieldWiperBottom.data != NULL && windshieldWiperTop.data != NULL) {
        Mat wiperBottomResized;
        cv::resize(windshieldWiperBottom, wiperBottomResized,
                   cv::Size(windshieldWiperBottom.cols * wiperMultiplier,
                            windshieldWiperBottom.rows * wiperMultiplier),
                   0, 0, cv::INTER_NEAREST);
        double bottomWiperLeftEdge = frame_with_mask.cols /
                                     2; //measured from left edge of screen. It's a bit off center (center would be frame_with_mask.cols/2-wiperBottomResized.cols/2) so that it doesn't overlap with the road lines.
        double bottomWiperTopEdge = frame_with_mask.rows - wiperBottomResized.rows -
                                    10; //measured from top edge of screen
        //copyWithPixelTransparencyUsingAlpha(frame_with_mask, wiperBottomResized, bottomWiperLeftEdge, bottomWiperTopEdge, wiperBottomResized.cols, wiperBottomResized.rows, 250);
        drawImageOptimized(frame_with_mask, wiperBottomResized, bottomWiperLeftEdge, bottomWiperTopEdge);

        Mat wiperTopResized;
        cv::resize(windshieldWiperTop, wiperTopResized,
                   cv::Size(windshieldWiperTop.cols * wiperMultiplier,
                            windshieldWiperTop.rows * wiperMultiplier),
                   0, 0, cv::INTER_NEAREST);
        
        Point2f pt(wiperTopResized.cols / 2., wiperTopResized.rows / 2.);
        double degrees = isParamNoExerciseDone(angle) ? 0 : -angle; //in degrees, not radians. Positive values mean counterclockwise.
        Mat rotMat = getRotationMatrix2D(pt, degrees, 1.0);
        warpAffine(wiperTopResized, wiperTopResized, rotMat,
                   Size(wiperTopResized.cols, wiperTopResized.rows));
        cv::Rect myROI(0, 0, wiperTopResized.cols, (int) (wiperTopResized.rows * 0.6));
        Mat croppedImage = wiperTopResized(myROI);
        /*copyWithPixelTransparencyUsingAlpha(frame_with_mask, croppedImage,
                                            frame_with_mask.cols / 2 - croppedImage.cols / 2,
                                            frame_with_mask.rows - wiperBottomResized.rows - 10 -
                                            croppedImage.rows * .8,
                                            croppedImage.cols, croppedImage.rows, 250); */
        
        int wiper_top_start_x = bottomWiperLeftEdge + wiperBottomResized.cols/2 - croppedImage.cols/2;
        int wiper_top_start_y = frame_with_mask.rows-wiperBottomResized.rows-10-croppedImage.rows*0.8;
        drawImageOptimized(frame_with_mask, croppedImage, wiper_top_start_x, wiper_top_start_y);
    }

    
    return angle;//frame_with_mask;
}
    
#ifdef EXTERN_C
}
#endif
