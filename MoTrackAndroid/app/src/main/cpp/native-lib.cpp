//
//  native-lib.h
//  MoTrackTestiOS/MoTrack Android
//
//  Created by Rahul Yerrabelli on 12/27/18 / 11/24/18
//  Copyright © 2018 MoTrack Therapy. All rights reserved.
//

//Use this to connect OpenCV C++
//https://stackoverflow.com/questions/38958876/can-opencv-for-android-leverage-the-standard-c-support-to-get-native-build-sup

//Must edit CameraBridgeViewBase to scale preview video properly!
//http://answers.opencv.org/question/8639/android-getting-largest-preview-image-possible/


//The calibration import must always be the first import in order to make use of device-dependent preprocessor directives
#include "calibration.h"


//device specific includes/imports
#if MY_OS==ANDROID_OS
    //Below are the imports for the android version
    #include <jni.h>
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
#include <cstdio>
#include <time.h>
#include <cstdlib>
#include <string>
#include <numeric>
#include <map>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <math.h>
#include "json.hpp"

#include "interactive_tutorial.h"
#include "native-lib.h"
#include "audio_player.h"
#include "motion_info.h"
#include "bilinear_approx.h"
#include "fist_analysis.h"
#include "hook_fist_analysis.h"
#include "pronation_analysis.h"
#include "fabduction_analysis.h"
#include "topposition_analysis.h"
#include "deviation_windshield.h"
#include "deviation_catapult.h"
#include "flexion_knock.h"
#include "flexion_cliff.h"
#include "pronation_dial.h"
#include "pronation_ketchup.h"
#include "fist_paddle.h"
#include "fabduction_peacock.h"
#include "topposition_alligator.h"
#include "hook_fist_caterpillar.h"


using namespace cv;
using namespace std;
using json = nlohmann::json;

#ifdef EXTERN_C
extern "C" {
#endif

#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
extern "C" {

void
Java_com_motracktherapy_motrack_CVActivity_setCurveArrows(JNIEnv *env, jobject, long addrLeft, long addrRight, jboolean jincludes_alpha) {
    if (addrLeft != -1 && addrRight != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (includes_alpha) {
            /* The following is needed for png images with an alpha channel */
            // process input image
            cv::Mat *leftArrow = (cv::Mat *) addrLeft;
            cv::Mat *rightArrow = (cv::Mat *) addrRight;
            // curently image is in bgra. Convert to rgba
            cv::cvtColor(*leftArrow, leftCurveArrow, cv::COLOR_BGRA2RGBA);
            //cv::cvtColor(*rightArrow, rightCurveArrow, cv::COLOR_BGRA2RGBA);
            //If you don't want to convert and keep the original colorspace, you can use this
            //lemonBottle = *lemonImage;
        } else {
            /* The following is needed for jpg images or for png images without an alpha channel
             // process input image
             cv::Mat* lemonImage = (cv::Mat*)addrLemonImage;
             // curently image is in bgr, convert to bgra
             Mat lemonBgra(lemonImage->rows, lemonImage->cols, CV_8UC4);
             cv::cvtColor(*lemonImage, lemonBgra, cv::COLOR_BGR2BGRA);
             // rearrange the channels to rgbate
             cv::Mat lemonRgba(lemonBgra.size(), lemonBgra.type());
             int from_to[] = { 0,2, 1,1, 2,0, 3,3 };
             cv::mixChannels(&lemonBgra,1,&lemonRgba,1,from_to,4);
             lemonBottle = setPixelTransparent(lemonRgba,255,255,255);
             */
        }
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setSizeRefImg(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        //bool includes_alpha = (bool) jincludes_alpha;
        //if (includes_alpha) {
            /* The following is needed for png images with an alpha channel */
            // process input image
            cv::Mat *img = (cv::Mat *) addr;
            // curently image is in bgra. Convert to rgba
            cv::cvtColor(*img, sizeRefImgOrig, cv::COLOR_BGRA2RGBA);
            if (whichHand == Hand::LEFT) {
                cv::flip(sizeRefImgOrig, sizeRefImgOrig, +1);
            }
            sizeRefImg = sizeRefImgOrig.clone();
            sizeRefImgAlpha.data = NULL;

            //If you don't want to convert and keep the original colorspace, you can use this
            //lemonBottle = *lemonImage;
        //} else {
            /* The following is needed for jpg images or for png images without an alpha channel
             // process input image
             cv::Mat* lemonImage = (cv::Mat*)addrLemonImage;
             // curently image is in bgr, convert to bgra
             Mat lemonBgra(lemonImage->rows, lemonImage->cols, CV_8UC4);
             cv::cvtColor(*lemonImage, lemonBgra, cv::COLOR_BGR2BGRA);
             // rearrange the channels to rgbate
             cv::Mat lemonRgba(lemonBgra.size(), lemonBgra.type());
             int from_to[] = { 0,2, 1,1, 2,0, 3,3 };
             cv::mixChannels(&lemonBgra,1,&lemonRgba,1,from_to,4);
             lemonBottle = setPixelTransparent(lemonRgba,255,255,255);
             */
        //}
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setMLNet(JNIEnv *env, jobject, long addr) {
    if (addr != -1) {
        cv::dnn::Net *myNet = (cv::dnn::Net *) addr;
        net = *myNet;
        int myint = 2+2;
    } else {
        cout << "Addr is null";
    }
}
    
// Get Methods
int
Java_com_motracktherapy_motrack_CVActivity_getCurrentMotion(JNIEnv *env, jobject) {
    return currentMotion;
}

int
Java_com_motracktherapy_motrack_CVActivity_getSkips(JNIEnv *env, jobject) {
    return motionsMap.motions[currentMotion].numSkips;
}

int
Java_com_motracktherapy_motrack_CVActivity_getNumCompleted(JNIEnv *env, jobject, jint mot) {
    return motionsMap.motions[mot].numRepsCompleted;
}

double
Java_com_motracktherapy_motrack_CVActivity_getGameScore(JNIEnv *env, jobject) {
    return getGameScore();
}

int
Java_com_motracktherapy_motrack_CVActivity_getNumRepsCompletedTot(JNIEnv *env, jobject) {
    return getNumRepsCompletedTot();
}

jstring
Java_com_motracktherapy_motrack_CVActivity_getDescription(JNIEnv *env, jobject) {
    return env->NewStringUTF(motionsMap.motions[currentMotion].motionInstructions.c_str());
}

int
Java_com_motracktherapy_motrack_CVActivity_getMotionTitle(JNIEnv *env, jobject) {
    return currentMotion;
}

int
Java_com_motracktherapy_motrack_CVActivity_getDoneState(JNIEnv *env, jobject) {
    return getDoneState();
}

bool
Java_com_motracktherapy_motrack_CVActivity_getDoneInteractiveTutorial(JNIEnv *env, jobject) {
    return getDoneInteractiveTutorial();
}

jstring
Java_com_motracktherapy_motrack_CVActivity_getCrashErrorDescription(JNIEnv *env, jobject) {
    //return getCrashErrorDescription();
    return env->NewStringUTF(getCrashErrorDescription().c_str());
}
jstring
Java_com_motracktherapy_motrack_CVActivity_getCrashErrorDescriptionCoded(JNIEnv *env, jobject) {
    //return getCrashErrorDescription();
    return env->NewStringUTF(getCrashErrorDescriptionCoded().c_str());
}

//Source: https://stackoverflow.com/questions/11558899/passing-a-string-to-c-code-in-android-ndk
std::string ConvertJString(JNIEnv* env, jstring str) {
    //if ( !str ) LString();

    const jsize len = env->GetStringUTFLength(str);
    const char* strChars = env->GetStringUTFChars(str, (jboolean *)0);

    std::string Result(strChars, len);

    env->ReleaseStringUTFChars(str, strChars);

    return Result;
}

/*
 * Initializes variables
 */
void Java_com_motracktherapy_motrack_CVActivity_initialize(
        JNIEnv *env,
        jobject, /* this */
        jint jnrtbc_each, jint hand, jstring json_data) {
    std::string json_data_str = ConvertJString( env, json_data );
    initialize((int) jnrtbc_each, (int) hand, json_data_str);
}
/*
 * Toggles learning rate
 * Note: the iOS version is in calibration.cpp, not native-lib
 */
void
Java_com_motracktherapy_motrack_CVActivity_setLearning(JNIEnv *env, jobject, jboolean jlearn) {
    bool learn = (bool) jlearn;
    setLearningRate(learn);
}

/*
 * Set's the amount of extra info to display. This is used for better testing.
 */
void
Java_com_motracktherapy_motrack_CVActivity_setDisplayExtraInfo(JNIEnv *env, jobject, jint jdisplayExtraInfoAmount) {
    setDisplayExtraInfo((int) jdisplayExtraInfoAmount);
}

/*
 * Increments skip
 * @return 1 if currentMotion was incremented, 0 otherwise
 */
int
Java_com_motracktherapy_motrack_CVActivity_doSkip(JNIEnv *env, jobject, jboolean jlearn) {
    return doSkip();

}

/*
 * Toggles show background
 */
void
Java_com_motracktherapy_motrack_CVActivity_setShowingBackground(JNIEnv *env, jobject, jint jshow) {
    showbackground = (int) jshow;
}

/*
 * Tells java whether game should be landscape or portrait depending on the game
 */
int
Java_com_motracktherapy_motrack_CVActivity_getOrientationForExerciseGame(JNIEnv *env, jobject, jint jexercise_game) {
    return getOrientationForExerciseGame( (int) jexercise_game );
}
    
/*
 * Sets which game/exercise to do and which orientatiion the screen should be in
 */
void
Java_com_motracktherapy_motrack_CVActivity_setExerciseGameAndOrientation(JNIEnv *env, jobject, jint jexercise_game, jint jorientation) {
    setExerciseGameAndOrientation((int) jexercise_game, (int) jorientation);
}




/*
 * Takes image, preprocesses, analyzes, and displays
 * Input frameDisplay is in RGBA (not BGRA which is OpenCV's default for opencv image reading) because the images are read by native readers in Android and iOS.
 */
jboolean
Java_com_motracktherapy_motrack_CVActivity_inputImage(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight, jobject srcBuffer,
        jobject dstSurface, jboolean testing, jboolean jDoInteractiveTutorial) {

    bool isTesting = testing;
    bool doInteractiveTutorial = jDoInteractiveTutorial;
    //prepare the mat and native window
    uint8_t *srcLumaPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(srcBuffer));

    //setFeedbackTextFromC(env, obj, "this is my test");
    if (srcLumaPtr == nullptr) {
        return JNI_FALSE;
    }


    cv::Mat mYuv(srcHeight + srcHeight / 2, srcWidth, CV_8UC1, srcLumaPtr);

    int dstWidth = srcHeight;
    int dstHeight = srcWidth;

    ANativeWindow *win = ANativeWindow_fromSurface(env, dstSurface);
    ANativeWindow_acquire(win);

    ANativeWindow_Buffer buf;


    ANativeWindow_setBuffersGeometry(win, dstWidth, dstHeight, 0); /*format unchanged*/

    if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
        ANativeWindow_release(win);
        return JNI_FALSE;
    }

    uint8_t *dstLumaPtr = reinterpret_cast<uint8_t *>(buf.bits);
    Mat dstRgba(dstHeight, buf.stride, CV_8UC4,
                dstLumaPtr);        // TextureView buffer, use stride as width
    Mat srcRgba(srcHeight, srcWidth, CV_8UC4);

    // convert YUV -> RGBA
    cv::cvtColor(mYuv, srcRgba, COLOR_YUV2BGRA_NV21);

    bool portrait_intended = orientation_game == 0;
    int frameWidth = (portrait_intended ? dstWidth : srcWidth); // ( ? : ) is called ternary operator
    int frameHeight = (portrait_intended ? dstHeight : srcHeight);
    Mat frameDisplay = Mat(frameHeight, frameWidth, CV_8UC4); // actually displayed

    bool calibResult = true; //whether or not lighting/exposure can be changed


    try { //catch all c++ errors
        Mat fgmask;
        Mat frame_with_mask;
        Mat frameEdit; // all the processing done on this - smaller so faster

        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
        
        double ratio = 100.0 / frameDisplay.cols;
        //double ratio = exercise_game==102 ? 1/0 : 100.0 / frameDisplay.cols; //i.e. make cols (width) equal to 300


        if (portrait_intended) { //Rotate & resize
            if (textWriteRectangle.width != frameWidth) {
                //textWriteRectangle = cv::Rect(0,0,frameWidth,int(frameHeight/5.5));
                textWriteRectangle = cv::Rect(0,0,frameWidth,int( frameHeight*0.2));

                /*
                 * Below code can give more space when instructions are multiple lines
                std::string s = motionsMap.motions[currentMotion].motionInstructions;
                char ch = ';';
                int count = std::count(s.begin(), s.end(), ch);
                textWriteRectangle = cv::Rect(0,0,frameWidth,int( frameHeight*(0.2+0.0*std::min(count,3) )));
                 */
            }
            // Rotate 90 degree
            cv::transpose(srcRgba, frameDisplay);
            cv::flip(frameDisplay, frameDisplay, 0);
            cv::flip(frameDisplay, frameDisplay, 1);

            cv::resize(frameDisplay, frameEdit, cv::Size(), ratio, ratio, cv::INTER_NEAREST);
        } else {
            if (textWriteRectangle.height != frameHeight) {
                textWriteRectangle = cv::Rect(0,0,int(frameHeight/3.5),frameHeight);
            }
            // For landscape, rotate after not before. That way, the image is sideways for the image.
            // Thus, just set the frame to be srcRgba, instead of writing the rotated version of srcRgba to it.
            cv::flip(srcRgba, frameDisplay, 1);
            cv::resize(frameDisplay, frameEdit, cv::Size(), ratio, ratio, cv::INTER_NEAREST);

        }


        if (greenTint.cols != frameDisplay.cols) {
            greenTint = cv::Mat::zeros(frameDisplay.size(),frameDisplay.type());
            greenTint.setTo(cv::Scalar(0,255,0));
        }

        Mat originalIm = frameDisplay.clone();
        //inputImageDimChanged =  ( (frameEdit.cols==inputImageWidth) && (frameEdit.rows==inputImageHeight));
        //inputImageWidth  = frameEdit.cols;
        //inputImageHeight = frameEdit.rows;
        preprocess(frameEdit, frameDisplay, fgmask, ratio, outlineColor);//, frame_with_mask);
        if (doInteractiveTutorial) {
            doneInteractiveTutorial = runInteractiveTutorial(frameDisplay, portrait_intended,  fgmask,  ratio);
        } else {
            int calibInt = runTimedCalibration(isTesting, frameDisplay, portrait_intended, doInteractiveTutorial);
            calibResult =
                    ((calibInt == STAGE_GAMEPLAY) || (calibInt == STAGE_BG_SUBTRACT_STANDARD) ||
                     (calibInt == STAGE_REDBOX1)) && (fgLocAlg < 3000 || fgLocAlg >=
                                                                         5000); //if fgLocAlg is not for the ML algorithms,
            if (calibInt == STAGE_GAMEPLAY) {
                if (badBackground) {
                    showbackground = 0;
                } else {
                    showbackground = 3;
                }
            } else if (calibInt == STAGE_REDBOX1) {
                showbackground = 0;
            }

            //bool isCorrect = false;
            if (portrait_intended) {
                if (showbackground >= 1) {
                    outlineColor = analyzeAndDisplay(frameEdit, frameDisplay, fgmask,
                                                     frame_with_mask, ratio);
                } else {
                    frame_with_mask = frameDisplay;
                }

            } else {
                if (showbackground >= 1) {
                    outlineColor = analyzeAndDisplay(frameEdit, frameDisplay, fgmask,
                                                     frame_with_mask, ratio);
                    //analyzeAndDisplay(frame, fgmask, frame_with_mask);
                } else {
                    // For some reason, the border only shows up if you do
                    // frame_with_mask = frame.clone();
                    // NOT
                    // frame_with_mask = frame;
                    frame_with_mask = frameDisplay.clone();
                }

            }

            //double alpha = 1; //increase contrast if this is greater than 1
            //double beta = -127*(alpha-1); //-127*(alpha-1) is enough to offset contrast, anymore/less will cause saturation
            //foreground.convertTo(foreground, -1, alpha, beta); // new_image = alpha*image + beta; used to increase contrast/saturate
            //addWeighted(frame_with_mask,1.0,frameDisplay,0.25,0,frameDisplay); //output into frame_with_mask
            //ensureHandAtTopLevel(frameDisplay, fgmask, originalIm);
            /*
            //Draws border rectangle to test if border is being cutoff
            Scalar value( 250, 0, 250 );
            cv::Rect rect(0, 0, frame_with_mask.cols, frame_with_mask.rows);
            //114 is the min that won't be seen, 115 is visible in landscape
            //284 is the minimum that you can still see something
            cv::rectangle(frame_with_mask, rect, value, 40, 8);
            */

            //Test Crash
            //cv::copyMakeBorder(frameDisplay, frameDisplay, -10, -5, -60, 0, BORDER_CONSTANT, CLR_BLACK_4UC);


            if (calibInt == STAGE_REDBOX1) {
                //higher speedReq is faster
                int speedReq = 2;
                addHandForSizeReference(frameDisplay, portrait_intended, speedReq, false);
            }

            //draw image resolution for testing
            if (displayExtraInfoAmount > 1) {
                cv::putText(frameDisplay, std::to_string(frameDisplay.rows) + "x" +
                                          std::to_string(frameDisplay.cols),
                            cv::Point(30, frameDisplay.rows * 4.0 / 5), cv::FONT_HERSHEY_DUPLEX,
                            0.5, Scalar(0, 0, 0, 255), 1);
            }


            //Mat frame_to_display(dstHeight, dstWidth, CV_8UC4);
            /*if (!handContour.empty()) { // draw outline of hand
                // need to do this because of the way draw contours work
                vector<vector<cv::Point>> contourTopVector;
                contourTopVector.push_back(handContour);
                if (isCorrect) { // motion is correct, draw green
                    cv::drawContours(frameDisplay, contourTopVector, 0, Scalar(0, 255, 0, 255),
                                     4); //color is green
                } else { // motion is correct, draw red
                    cv::drawContours(frameDisplay, contourTopVector, 0, Scalar(255, 0, 0, 255),
                                     4); //color is red
                }
            }*/
        }
        if (useGreenTint > 0) {
            useGreenTint--;
            cv::addWeighted(greenTint,0.45,frameDisplay,0.55,0,frameDisplay);
        }
        if (portrait_intended) {
            //Mat frame_to_display = frame_with_mask;
            //cv::resize(frameDisplay, frameDisplay, cv::Size(frameWidth, frameHeight), 0,0, INTER_NEAREST );
        } else {
            // Rotate
            //cv::transpose(frame_with_mask, frame_to_display);
            cv::transpose(frameDisplay, frameDisplay);
            //cv::flip(frame_to_display, frame_to_display, 0);
            //cv::flip(frame_to_display, frame_to_display, 1);
            cv::flip(frameDisplay, frameDisplay, 1);
            //cv::resize(frame_to_display, frame_to_display, cv::Size(), 1/xratio,1/yratio, INTER_NEAREST );
        }
        
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
        chrono::steady_clock::time_point t3 = chrono::steady_clock::now();
        chrono::steady_clock::time_point t4 = chrono::steady_clock::now();
        chrono::steady_clock::time_point t5 = chrono::steady_clock::now();
        
        auto time_span10 = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        auto time_span20 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t0);
        auto time_span30 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0);
        auto time_span40 = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t0);
        auto time_span50 = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t0);
        cout << time_span10.count()/1000 << " " << time_span20.count()/1000 << " " << time_span30.count()/1000 << " " << time_span40.count()/1000 << " " << time_span50.count()/1000 << " " << "ms with fgLocAlg=" << fgLocAlg << endl;

    } catch (cv::Exception ex) { //cv::Exception is a subclass of std::exception, but for some reason, if you catch it as a std::exception, calling ex.what() only returns "std::exception" not a full description
        //Exception reference: https://stackoverflow.com/questions/8415032/catching-exceptions-thrown-from-native-code-running-on-android
        reportException( ex.what() );
        
    } catch (nlohmann::json::exception ex) { //Can happen if you try to receive an element not there
        reportException( ex.what() );
        
    } catch (std::exception ex) { //cv::Exception and nlohmann::json::exception are both examples of std::exception, but this works as more of a catch-all if there is an std::exception that isn't the above categories. Once again, can't just catch this exception because then ex.what() isn't as detailed.
        reportException( ex.what() );
        
    }
    
    if (appHasCrashed > 0) {
        Mat frameDisplayError = cv::Mat(frameDisplay.size(), CV_8UC4, CLR_MOTRACK_GRAY_4UC);
        int errorFontSize = std::max(frameDisplay.rows,frameDisplay.cols)*0.005;
        cv::putText(frameDisplayError, "ERROR", cv::Point(0, frameDisplay.rows/2.0), fontType,
                    errorFontSize, CLR_MOTRACK_BLUE_4UC, getThicknessFromFontScale(errorFontSize) );
        __android_log_print(ANDROID_LOG_ERROR,"ABC"  , "%s",crashErrorDescription.c_str() );

        addWeighted(frameDisplay, 0.1, frameDisplayError, 0.9, 0, frameDisplay);
    }


    // copy to TextureView surface
    uchar *dbuf;
    uchar *sbuf;
    dbuf = dstRgba.data;
    sbuf = frameDisplay.data;//flipRgba.data;
    int i;
    for (i = 0; i < frameDisplay.rows; i++) {
        dbuf = dstRgba.data + i * buf.stride * 4;
        memcpy(dbuf, sbuf, frameDisplay.cols * 4); //flipRgba.cols * 4);
        sbuf += frameDisplay.cols * 4;//flipRgba.cols * 4;
    }

    cout << sizeof(dbuf) << "," << sizeof(sbuf) << endl;
    ANativeWindow_unlockAndPost(win);
    ANativeWindow_release(win);
    mYuvlast = frameDisplay;

    // release
    /*frame_to_display.release();
    mYuv.release();
    dstRgba.release();
    srcRgba.release();
    frame.release();
    fgmask.release();
    frame_with_mask.release();*/
    return calibResult ? JNI_TRUE : JNI_FALSE;
}


void Java_com_motracktherapy_motrack_CVActivity_resetCalibrationBackground( JNIEnv *env, jobject obj) {
    resetCalibrationBackground();
}


/*
 * Takes image, preprocesses, analyzes, and displays
 */
/*void
Java_com_motracktherapy_motrack_CVActivity_redoImage(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight,
        jobject dstSurface) {

    int dstWidth;
    int dstHeight;

    ANativeWindow *win = ANativeWindow_fromSurface(env, dstSurface);
    ANativeWindow_acquire(win);

    ANativeWindow_Buffer buf;

    dstWidth = srcHeight;
    dstHeight = srcWidth;

    ANativeWindow_setBuffersGeometry(win, dstWidth, dstHeight, 0); //format unchanged

    if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
        ANativeWindow_release(win);
        return;
    }
    Mat flipRgba = mYuvlast;
    Mat fgmask;
    Mat frame_with_mask;

    if (showbackground == 1) {
        preprocess(flipRgba, fgmask);//, frame_with_mask);
        //preprocess(flipRgba, fgmask, frame_with_mask);
        analyzeAndDisplay(flipRgba, fgmask, frame_with_mask);
    } else {
        frame_with_mask = flipRgba;
    }

    uint8_t *dstLumaPtr = reinterpret_cast<uint8_t *>(buf.bits);
    Mat dstRgba(dstHeight, buf.stride, CV_8UC4,
                dstLumaPtr);        // TextureView buffer, use stride as width

    // copy to TextureView surface
    uchar *dbuf;
    uchar *sbuf;
    dbuf = dstRgba.data;
    sbuf = frame_with_mask.data;//flipRgba.data;
    int i;
    for (i = 0; i < flipRgba.rows; i++) {
        dbuf = dstRgba.data + i * buf.stride * 4;
        memcpy(dbuf, sbuf, frame_with_mask.cols * 4); //flipRgba.cols * 4);
        sbuf += frame_with_mask.cols * 4;//flipRgba.cols * 4;
    }

    ANativeWindow_unlockAndPost(win);
    ANativeWindow_release(win);
}*/

/*
 * This method is for testing purposes.  It will display the camera without alteration
 */
/*void
Java_com_motracktherapy_motrack_CVActivity_displayCamera(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight, jobject srcBuffer,
        jobject dstSurface) {

    //prepare the mat and native window
    uint8_t *srcLumaPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(srcBuffer));

    if (srcLumaPtr == nullptr) {
        return;
    }

    int dstWidth;
    int dstHeight;

    cv::Mat mYuv(srcHeight + srcHeight / 2, srcWidth, CV_8UC1, srcLumaPtr);

    ANativeWindow *win = ANativeWindow_fromSurface(env, dstSurface);
    ANativeWindow_acquire(win);

    ANativeWindow_Buffer buf;

    dstWidth = srcHeight;
    dstHeight = srcWidth;

    ANativeWindow_setBuffersGeometry(win, dstWidth, dstHeight, 0); // format unchanged

    if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
        ANativeWindow_release(win);
        return;
    }

    uint8_t *dstLumaPtr = reinterpret_cast<uint8_t *>(buf.bits);
    Mat dstRgba(dstHeight, buf.stride, CV_8UC4,
                dstLumaPtr);        // TextureView buffer, use stride as width
    Mat srcRgba(srcHeight, srcWidth, CV_8UC4);
    Mat flipRgba(dstHeight, dstWidth, CV_8UC4);

    // convert YUV -> RGBA
    cv::cvtColor(mYuv, srcRgba, CV_YUV2RGBA_NV21);
    //cv::cvtColor(mYuv, flipRgba, CV_YUV2RGBA_NV21);

    // Rotate 90 degree
    //cv::transpose(srcRgba, flipRgba);
    //cv::flip(flipRgba, flipRgba, 0);
    //cv::flip(flipRgba, flipRgba, 1);

    Mat fgmask;
    Mat frame_with_mask;

    preprocess(flipRgba, fgmask, frame_with_mask);
    //analyzeAndDisplay(flipRgba, fgmask, frame_with_mask);

    // copy to TextureView surface
    uchar *dbuf;
    uchar *sbuf;
    dbuf = dstRgba.data;
    sbuf = flipRgba.data;
    int i;
    for (i = 0; i < flipRgba.rows; i++) {
        dbuf = dstRgba.data + i * buf.stride * 4;
        memcpy(dbuf, sbuf, flipRgba.cols * 4);
        sbuf += flipRgba.cols * 4;
    }

    ANativeWindow_unlockAndPost(win);
    ANativeWindow_release(win);
}*/

// Not technically a "Java_com_..." function but included because it requires JNIEnv *env access
// and is specific to android
// This function's purpose is to set feedback text view to given text
void setFeedbackTextFromC( JNIEnv* env, jobject obj, std::string s ){
    // Construct a String
    jstring jstr = env->NewStringUTF(s.c_str());
    // First get the class that contains the method you need to call
    jclass clazz = env->GetObjectClass(obj);//FindClass("com/motracktherapy/motrack/CVActivity");
    // Get the method that you want to call
    //jmethodID messageMe = env->GetMethodID(clazz, "setFeedbackTextView", "(Ljava/lang/String;)V");
    // Call the method on the object
    //jobject result = env->CallObjectMethod(jstr, messageMe);
    // Get a C-style string
    //const char* str = env->GetStringUTFChars((jstring) result, NULL);
    // Clean up
    //env->ReleaseStringUTFChars(jstr, str);
    /*jclass cls = env->GetObjectClass(env, obj);
    jmethodID mid = env->GetMethodID(env, cls, "callback", "(I)V");
    if (mid == 0)
        return;
    printf("In C, depth = %d, about to enter Java\n", depth);
    (*env)->CallVoidMethod(env, obj, mid, depth);
    printf("In C, depth = %d, back from Java\n", depth);*/
}


} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */


int cppInt(int input) {
    return input + 5;
}
    
int shouldCameraBeInitializedAsContinuous(int input) { //iOS only
    return fgLocAlg >= 3000 && fgLocAlg < 5000;
}
    
void setSizeRefImg(cv::Mat picture) {
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    sizeRefImgOrig = picture;
    sizeRefImg = sizeRefImgOrig.clone();
    sizeRefImgAlpha.data = NULL;
}

void setCurveArrows(cv::Mat left, cv::Mat right) {
    leftCurveArrow  = left;
    rightCurveArrow = right;
}
    
cv::Mat drawDescriptionArea(cv::Mat frame_with_mask, int newWidth, int newHeight, int returnWidth, int returnHeight) {
    if (returnWidth != -1 && returnHeight != -1) {
        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();

        cv::Mat outputImage( (int) returnHeight, (int) returnWidth, frame_with_mask.type());
        bool resizeFully = false; //if true, then the image will be stretched in order to fit the screen. Otherwise, the aspect ratio will be kept and remaining whitespace will be left.
        
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();

        if (resizeFully) {
            cv::resize(frame_with_mask, outputImage, cv::Size((int)returnWidth, (int)returnHeight),    -1,    -1, cv::INTER_NEAREST);
            
        } else if (/* DISABLES CODE */ (true) || returnWidth*1.0/frame_with_mask.cols < returnHeight*1.0/frame_with_mask.rows) {
            
            outputImage.setTo(CLR_MOTRACK_GRAY_4UC);
            chrono::steady_clock::time_point t2 = chrono::steady_clock::now();

            if (newHeight<returnHeight) { //will throw an error if we set the image height to newHeight because newHeight is too big if
                //Use below line if you want the app all the way at the top (instead of centered) in portrait mode
                cv::Mat outputImageFilled = outputImage(cv::Rect(cv::Point(0, returnHeight-newHeight), cv::Point(returnWidth, returnHeight)));
                //cv::Mat outputImageFilled = outputImage(cv::Rect(cv::Point(0, (int) (returnHeight - newHeight)/2),
                //                                                 cv::Point((int) returnWidth, (int)(returnHeight + newHeight)/2 )));
                cv::resize(frame_with_mask, outputImageFilled, cv::Size((int)returnWidth, newHeight),    -1,    -1, cv::INTER_NEAREST);
                
                
                /*if ((double(returnHeight)-double(newHeight))/double(returnHeight) > 0.1) {
                 textWriteRectangle = cv::Rect(0,0,returnWidth,returnHeight-newHeight);
                 shouldWriteOnIm = false;
                 }*/
                if ((double(returnHeight)-double(newHeight))/double(returnHeight) > 0.1) {
                    cv::line(outputImage,cv::Point(0,textWriteRectangle.height),cv::Point(outputImage.cols,textWriteRectangle.height),cv::Scalar(0,0,0,255),5);
                    // write text from buffer
                    for (vector< tuple<string, double, cv::Point,cv::Scalar> > ::const_iterator i = bufferForWriting.begin(); i != bufferForWriting.end(); ++i) {
                        cv::putText(outputImage, get<0>(*i), get<2>(*i), fontType,
                                    get<1>(*i),
                                    cv::Scalar(0, 0, 0, 255), 2 + 4); //black outside
                        cv::putText(outputImage, get<0>(*i), get<2>(*i), fontType,
                                    get<1>(*i), get<3>(*i), 2);
                    }
                    if ( exercise_game<=0 || (fgLocAlg>=0 && (displayExtraInfoAmount > 0 || (fgLocAlg != 0 && fgLocAlg != 4 && fgLocAlg != 1674 && fgLocAlg != 1684 && fgLocAlg != 3000))) )  { //don't display the fgLocAlg if it is both the standard algorithm and displayExtraInfoAmount is 0
                        //Display fgLocAlg, the background subtraction algorithm
                        string fgLocAlgStr = "";
                        if (fgLocAlg == 4) fgLocAlgStr = "Alg: Standard";
                        else if (fgLocAlg == 1684) fgLocAlgStr = "Alg: Ambient";
                        else if (fgLocAlg >= 3000 && fgLocAlg < 5000) fgLocAlgStr = "Alg: AI-based";
                        else fgLocAlgStr = "Alg. #"+std::to_string(fgLocAlg);
                        cv::putText(outputImage, fgLocAlgStr, cv::Point(30, 30), cv::FONT_HERSHEY_DUPLEX, 1.0, Scalar(0, 0, 0, 255), 2);
                    }
                }
                
                chrono::steady_clock::time_point t3 = chrono::steady_clock::now();
                
                auto time_span10 = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
                auto time_span20 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t0);
                auto time_span30 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0);
                cout << time_span10.count()/1000 << " " << time_span20.count()/1000 << " " << time_span30.count()/1000 << " ms || ";
                
            } else {
                cv::line(outputImage,cv::Point(textWriteRectangle.width,0),cv::Point(textWriteRectangle.width,outputImage.rows),cv::Scalar(0,0,0,255),5);
                // move image to the right
                cv::Mat outputImageFilled = outputImage(cv::Rect(cv::Point(returnWidth - newWidth, 0), cv::Point(returnWidth, returnHeight)));
                
                //cv::Mat outputImageFilled = outputImage(cv::Rect(cv::Point((int)(returnWidth - newWidth)/2, 0),
                //                                                 cv::Point((int)(returnWidth + newWidth)/2, (int) returnHeight)));
                cv::resize(frame_with_mask, outputImageFilled, cv::Size(newWidth, (int) returnHeight),    -1,    -1, cv::INTER_NEAREST);
                
                //cv::putText(outputImage, "Alg:"+std::to_string(fgLocAlg), cv::Point(30, 30),cv::FONT_HERSHEY_DUPLEX, 1.0, Scalar(0, 0, 0, 255),2);
                
                if ((double(returnWidth)-double(newWidth))/double(returnWidth) > 0.1) {
                    for (vector< tuple<string, double, cv::Point,cv::Scalar> > ::const_iterator i = bufferForWriting.begin(); i != bufferForWriting.end(); ++i) {
                        
                        cv::putText(outputImage, get<0>(*i), get<2>(*i), fontType,
                                    get<1>(*i),
                                    cv::Scalar(0, 0, 0, 255), 2 + 4); //black outside
                        cv::putText(outputImage, get<0>(*i), get<2>(*i), fontType,
                                    get<1>(*i), get<3>(*i), 2);
                    }
                }
            }
            
        } else {
            
        }
        
        return outputImage;
    } else {
        return frame_with_mask;
    }

}
    
std::chrono::steady_clock::time_point lightsTutorialStartTime;
bool lightTutorialDone = false;

void initializeLightsTutorial(string json_data_str) {
    lightTutorialDone = false;
    lightsTutorialStartTime = std::chrono::steady_clock::now();
}

bool getLightsTutorialDoneState() {
    return lightTutorialDone;
}

cv::Mat doLightsTutorial(cv::Mat frameDisplay, int returnWidth, int returnHeight) {
    //get time count from the start of the tutorial
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration time_span = nowTime - lightsTutorialStartTime;
    double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;

    //get smaller version of the input image so that it can be analyzed in realtime
    Mat toReturn = frameDisplay.clone();
    double ratio = 90.0 / frameDisplay.cols;
    cv::Mat frameEdit;
    cv::resize(frameDisplay, frameEdit, cv::Size(), ratio, ratio, cv::INTER_NEAREST);
    
    //find the lights
    Mat frameEditHSV;
    cv::cvtColor(frameEdit, frameEditHSV, COLOR_RGB2HSV);
    vector<Mat> hsv;
    split(frameEditHSV,hsv);
    Mat my_light_mask, my_sat_mask;
    cv::threshold(hsv[2], my_light_mask, 250,255, THRESH_BINARY);
    cv::threshold(hsv[1], my_sat_mask, 150,255, THRESH_BINARY_INV);
    Mat combined_mask = my_light_mask & my_sat_mask;
    double percentLight = cv::countNonZero(combined_mask) *1.0/(combined_mask.rows*combined_mask.cols);
    
    //find the outline of the light areas
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(combined_mask, contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
    
    //resized the found light areas to the size of the original frame
    cv::resize(combined_mask, combined_mask, cv::Size(), 1/ratio, 1/ratio);
    for(int i1=0; i1<contours.size(); i1++) {
        for(int i2=0; i2<contours[i1].size(); i2++) {
            contours[i1][i2] = contours[i1][i2]/ratio;
        }
    }
    
    //draw the light areas
    toReturn.setTo(Scalar(255,0,0,255), combined_mask);
    for (int ind = 0; ind < contours.size(); ind++) {
        cv::drawContours(toReturn, contours, ind, Scalar(255,255,0,255), 5);
    }
    
    
    //Draw the text at the top that gives instructions
    int newHeight = (int) (returnHeight*frameDisplay.cols*1.0/frameDisplay.rows);
    int newWidth  = (int) (returnWidth*frameDisplay.rows*1.0/frameDisplay.cols);
    bufferForWriting.clear();
    
    vector<tuple<string,cv::Scalar>> toWrite;
    string lightFeedback;
    if (percentLight < 0.05) {
        lightFeedback = "This looks like a; great spot to do; your exercises!";
        lightTutorialDone = true;
    } else if (time_diff < 15.0) lightFeedback = "We'll outline any; areas that seem a; little too bright.";
    else lightFeedback = "You might need to get; up, move to another; area of the room, and; place your phone back; on a flat surface.";
    toWrite = splitTextUpBySemicolon(toWrite, lightFeedback, CLR_MOTRACK_BLUE_4UC);
    writeText_onRect(toReturn, toWrite);
    
    if (newHeight<returnHeight) { // portrait mode
        if ((double(returnHeight)-double(newHeight))/double(returnHeight) > 0.1) {
            textWriteRectangle = cv::Rect(0,0,returnWidth,returnHeight-newHeight);
            shouldWriteOnIm = false;
        }
    } else { // landscape mode
        if ((double(returnWidth)-double(newWidth))/double(returnWidth) > 0.1) {
            textWriteRectangle = cv::Rect(0,0,returnWidth-newWidth,returnHeight);
            shouldWriteOnIm = false;
        }
    }
    
    return drawDescriptionArea(toReturn, newWidth, newHeight, returnWidth, returnHeight);

}


void registerTap(double x, double y, int displayWidth, int displayHeight) {
    /*
    cout << "Tap at (X: " << x << ", Y: " << y << ")"  << endl;
    tapLocationXs.push_back(x);
    tapLocationYs.push_back(y);
    tapLocationsProcessed.push_back(false);
    tapStartTimes.push_back(std::chrono::steady_clock::now());*/
    
    y = y - (displayHeight - displayWidth*4.0/3.0);
    
    cout << "Tap at (X: " << x << ", Y: " << y << ")"  << endl;
    tapLocationXs.push_back(x);
    tapLocationYs.push_back(y);
    tapLocationsProcessed.push_back(false);
    tapStartTimes.push_back(std::chrono::steady_clock::now());
    
}

/*
 * Takes image, preprocesses, analyzes, and displays
 * Input frameDisplay is in RGBA (not BGRA which is OpenCV's default for opencv image reading) because the images are read by native readers in Android and iOS.
 */
std::tuple<cv::Mat,cv::Mat> inputImage(cv::Mat frameDisplay, int returnWidth, int returnHeight, bool isTesting, bool doInteractiveTutorial) {
    Mat fgmask;

    try { //catch all c++ errors
        bool returnInput = false; //would only be true for testing
        Mat inputImage;
        if (returnInput) inputImage = frameDisplay.clone();
        
        
        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();

        
        //cv::Mat frameDisplayOrig = frameDisplay.clone();
        
        if (greenTint.cols != frameDisplay.cols) {
            greenTint = cv::Mat::zeros(frameDisplay.size(),frameDisplay.type());
            greenTint.setTo(cv::Scalar(0,255,0));
        }
        
        Mat frame_with_mask;
        Mat frameEdit;
        double ratio = 90.0 / frameDisplay.cols;
        cv::resize(frameDisplay, frameEdit, cv::Size(), ratio, ratio, cv::INTER_NEAREST);
        int newHeight = (int) (returnHeight*frameDisplay.cols*1.0/frameDisplay.rows);
        int newWidth  = (int) ( returnWidth*frameDisplay.rows*1.0/frameDisplay.cols);
        bufferForWriting.clear();
        bool portrait_intended = newHeight<returnHeight;
        if (portrait_intended) { // portrait mode
            if ((double(returnHeight)-double(newHeight))/double(returnHeight) > 0.1) {
                textWriteRectangle = cv::Rect(0,0,returnWidth,returnHeight-newHeight);
                shouldWriteOnIm = false;
            }
        } else { // landscape mode
            if ((double(returnWidth)-double(newWidth))/double(returnWidth) > 0.1) {
                textWriteRectangle = cv::Rect(0,0,returnWidth-newWidth,returnHeight);
                shouldWriteOnIm = false;
            }
        }
        
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        chrono::steady_clock::time_point t2;
        preprocess(frameEdit, frameDisplay, fgmask, ratio, outlineColor);//, frame_with_mask);
        if (doInteractiveTutorial) {
            doneInteractiveTutorial = runInteractiveTutorial(frameDisplay, portrait_intended,  fgmask,  ratio);
            t2 = chrono::steady_clock::now();
            
        } else {
            int calibInt = runTimedCalibration(isTesting, frameDisplay,portrait_intended, doneInteractiveTutorial);
            bool calibResult = (calibInt == STAGE_GAMEPLAY) || (calibInt == STAGE_BG_SUBTRACT_STANDARD) || (calibInt == STAGE_REDBOX1);

            if (calibInt == STAGE_GAMEPLAY) {
                if (badBackground) {
                    showbackground = 0;
                } else {
                    showbackground = 3;
                }
            } else if (calibInt == STAGE_REDBOX1) {
                showbackground = 0;
            }
            
            
            //bool isCorrect = false;
            if (showbackground >= 1) {
                outlineColor = analyzeAndDisplay(frameEdit,frameDisplay,fgmask,frame_with_mask,ratio);//(frame, fgmask, frame_with_mask);
                frame_with_mask = frameDisplay;
            } else {
                frame_with_mask = frameDisplay;
            }
            t2 = chrono::steady_clock::now();
            if (calibInt == STAGE_REDBOX1) {
                //higher speedReq is faster
                int speedReq = 2; //(time_span20.count()/1000.0 > 10) ? 2 : 0;
                addHandForSizeReference(frameDisplay, portrait_intended, speedReq, false);
            }
            
            if (returnInput && !(calibInt == STAGE_REDBOX1)) frame_with_mask = inputImage;

        }
        
        chrono::steady_clock::time_point t3 = chrono::steady_clock::now();


        if (useGreenTint > 0) {
            useGreenTint--;
            //cout << frameDisplay.rows << " " << frameDisplay.rows << " " << frameDisplay.channels() << " ; " << greenTint.rows << " " << greenTint.rows << " " << greenTint.channels() << endl;
            cv::addWeighted(greenTint,0.25,frameDisplay,0.75,0,frameDisplay);
        }
        //if you want to test to make sure that nothing is cutoff, use this code to create a border
        /*cv::Rect border(cv::Point(0, 0), frame_with_mask.size());
        cv::Scalar color(255, 255, 0);
        int thickness = 10;
        cv::rectangle(frame_with_mask, border, color, thickness);*/

         /*if (!handContour.empty()) { // draw outline of hand
            // need to do this because of the way draw contours work
            vector<vector<cv::Point>> contourTopVector;
            contourTopVector.push_back(handContour);
            if (isCorrect) { // motion is correct, draw green
                cv::drawContours(frameDisplay, contourTopVector, 0, Scalar(0, 255, 0, 255),
                                 4); //color is green
            } else { // motion is correct, draw red
                cv::drawContours(frameDisplay, contourTopVector, 0, Scalar(255, 0, 0, 255),
                                 4); //color is red
            }
        }*/
        
        //frame_with_mask = frameDisplayOrig;
    
        
        if (!tapLocationXs.empty() || !tapLocationYs.empty()) {
            std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
            for (size_t i = 0; i < tapLocationXs.size(); i++) {
                std::chrono::steady_clock::duration time_span = nowTime - tapStartTimes[i];
                double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
                
                double display_tap_time = 0.25; //seconds
                if (time_diff <= display_tap_time) {
                    int radius = std::max(frameDisplay.rows,frameDisplay.cols)*0.05*(1-time_diff/display_tap_time);
                    cv::circle(frameDisplay, cv::Point(tapLocationXs[i],tapLocationYs[i]), radius*1.05, CLR_BLACK_4UC, -1);
                    cv::circle(frameDisplay, cv::Point(tapLocationXs[i],tapLocationYs[i]), radius, CLR_MOTRACK_BLUE_4UC, -1);
                    
                } else {
                    //tapLocationXs.clear();
                    //tapLocationYs.clear();
                    //tapStartTimes.clear();
                    
                    //delete the ith element
                    tapLocationXs.erase(tapLocationXs.begin() + i);
                    tapLocationYs.erase(tapLocationYs.begin() + i);
                    tapStartTimes.erase(tapStartTimes.begin() + i);
                }
            }

        }

        chrono::steady_clock::time_point t4 = chrono::steady_clock::now();

        if (colorMode == 1) {
            //cv::cvtColor(frame_with_mask, frame_with_mask, COLOR_RGBA2GRAY);
            //cv::cvtColor(frame_with_mask, frame_with_mask, COLOR_GRAY2RGBA); //Apparently not necessary.
            
            //applyColorMap(frame_with_mask, frame_with_mask, COLORMAP_JET);
            
            cv::cvtColor(frame_with_mask, frame_with_mask, COLOR_RGBA2RGB);
            applyColorMap(frame_with_mask, frame_with_mask, COLORMAP_RAINBOW);
            cv::cvtColor(frame_with_mask, frame_with_mask, COLOR_RGB2RGBA);
        }
        
        chrono::steady_clock::time_point t5 = chrono::steady_clock::now();
        frameDisplay = drawDescriptionArea(frame_with_mask, newWidth, newHeight, returnWidth, returnHeight);

        //auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        //cout << time_span.count()/1000 << "ms with fgLocAlg=" << fgLocAlg << endl;
        
        auto time_span10 = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        auto time_span20 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t0);
        auto time_span30 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0);
        auto time_span40 = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t0);
        auto time_span50 = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t0);
        cout << time_span10.count()/1000 << " " << time_span20.count()/1000 << " " << time_span30.count()/1000 << " " << time_span40.count()/1000 << " " << time_span50.count()/1000 << " " << "ms with fgLocAlg=" << fgLocAlg << endl;

    } catch (cv::Exception ex) { //cv::Exception is a subclass of std::exception, but for some reason, if you catch it as a std::exception, calling ex.what() only returns "std::exception" not a full description
        //Exception reference: https://stackoverflow.com/questions/8415032/catching-exceptions-thrown-from-native-code-running-on-android
        reportException( ex.what() );
        
    } catch (nlohmann::json::exception ex) { //Can happen if you try to receive an element not there
        reportException( ex.what() );
        
    } catch (std::exception ex) { //cv::Exception and nlohmann::json::exception are both examples of std::exception, but this works as more of a catch-all if there is an std::exception that isn't the above categories. Once again, can't just catch this exception because then ex.what() isn't as detailed.
        reportException( ex.what() );
    }
    
    if (appHasCrashed > 0) {
        Mat frameDisplayError = cv::Mat(frameDisplay.size(), CV_8UC4, CLR_MOTRACK_GRAY_4UC);
        int errorFontSize = std::max(frameDisplay.rows,frameDisplay.cols)*0.005;
        cv::putText(frameDisplayError, "Error", cv::Point(0, frameDisplay.rows/2.0), fontType,
                    errorFontSize, CLR_MOTRACK_BLUE_4UC, getThicknessFromFontScale(errorFontSize) );
        
        addWeighted(frameDisplay, 0.1, frameDisplayError, 0.9, 0, frameDisplay);
    }
    
    return std::make_tuple(frameDisplay, fgmask);
    
}

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX




 * ------------------------------------------------------------------------------------------------
 *                                  EXTERN-DEFINED VARIABLES Set
 *                   Set values for variables defined as extern on native-lib.h
 * ------------------------------------------------------------------------------------------------
 */
//bool hasBeenCalibrated = false; // when true, the background has already been calibrated
int orientation_game = -5; // -5 is invalid, 0 for portrait, 1 for landscape (with phone left on bottom)
int motions_done = 0; //0 means none are done; this variable was previously named done_state
int prevWindshieldAngle = 0;
//double holdExerciseLength = 1.0; // the length of time to hold an exercise
int outlineColor = 0; // describes color of hand outline (-1 red, 0 white, 1 green)
cv::Mat greenTint;

int appHasCrashed = 0;
std::string crashErrorDescription = "";

int currentMotion;
cv::Mat mYuvlast;
double game_score = 0;
std::string infoOutput = "", feedback = "";
//startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());


//auto startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
/*cv::Mat lemonBottle;
cv::Mat cup;
cv::Mat cupResized;*/



/*
 * ------------------------------------------------------------------------------------------------
 *                       Helper methods used in image analysis & image modification
 * ------------------------------------------------------------------------------------------------
 */

// Takes in input Mat with channels rgba, color value for red, blue, and green
// Sets all pixels with that color value to transparent
Mat setPixelTransparent(Mat& input_rgba, int r, int g, int b) {
    // find all pixels of interest and set alpha value to zero:
    for (int y = 0; y < input_rgba.rows; ++y) {
        for (int x = 0; x < input_rgba.cols; ++x) {
            cv::Vec4b &pixel = input_rgba.at<cv::Vec4b>(y, x);
            // if pixel is of color of interest
            if (pixel[0] == r && pixel[1] == g && pixel[2] == b) {
                // set alpha to zero:
                pixel[3] = 0;
            }
        }
    }
    return input_rgba;
}



void resetCalibrationBackground() {
    playAudioFileClick();
    resetCalibration();
}


//Controls automatic calibration for the first few seconds after opening a camera game
// if testing or already calibrated (past the "Remove your head... " part, then returns 0
// if still in calibrated part but past the first second, return 1
// if still in calibrated part and in first second, return 2
int runTimedCalibration(bool isTesting, Mat frameDisplay, bool portrait_intended, bool shouldRunTimedCalib) {
    if (!isTesting) { //} && !hasBeenCalibrated) {
        int timeCalib = timedBackgroundRemoval(frameDisplay, false, portrait_intended);
        if (timeCalib >= 5) {
            /*if (timeCalib <= 10) {
                //writeText(frame, "Now put your hand over the camera and try to perform the exercises. Don't move the phone!");
            }*/
            showbackground = 3;

            //hasBeenCalibrated = true;

            if (timeCalib <= 10) {
                currentStage = STAGE_REDBOX1;
                return currentStage;
            } else {
                currentStage = STAGE_GAMEPLAY;
                return currentStage;
            }
            STAGE_GAMEPLAY;
        } else {
            showbackground = 0;
            if (timeCalib >= 1) {
                currentStage = STAGE_BG_SUBTRACT_STANDARD;
                return currentStage;
            } else {
                currentStage = STAGE_BG_SUBTRACT_BEGINNING;
                return currentStage;
            }
            //return (timeCalib >= 1);
        }
    } else {
        currentStage = STAGE_GAMEPLAY;
        return currentStage;
    }
    
}
/*
 * Converts Mat object to 2D vector
 */
vector<vector<uchar>> toVec(const Mat matIn) {
    vector<vector<uchar>> vecOut(matIn.rows);
    for (int i = 0; i < matIn.rows; ++i) {
        vecOut[i].resize(matIn.cols);
        for (int j = 0; j < matIn.cols; ++j) {
            vecOut[i][j] = matIn.at<uchar>(i, j);
        }
    }
    return vecOut;
}

/*
 * Converts Mat object to 1D vector of points
 */
/*vector<cv::Point> toVec1D(const Mat matIn) {
    vector<cv::Point> vecOut(matIn.rows*matIn.cols);
    for (int i = 0; i < matIn.rows; ++i) {
        for (int j = 0; j < matIn.cols; ++j) {
            vecOut[i][j] = matIn.at<uchar>(i, j);
        }
    }
    return vecOut;
}*/

/*
 * Returns positions of nonzero elements (for uchar vectors)
 */
vector<int> non_zero__coord_uchar(vector<uchar> arr) {
    vector<int> nonzero;
    int i = 0;
    for (vector<uchar>::iterator it = arr.begin(); it != arr.end(); it++, i++) {
        if (*it != 0) { nonzero.push_back(i); }
    }
    return nonzero;
}

/*
 * Returns positions of nonzero elements (for int vectors)
 */
vector<int> non_zero_coord_int(vector<int> arr) {
    vector<int> nonzero;
    int i = 0;
    for (vector<int>::iterator it = arr.begin(); it != arr.end(); it++, i++) {
        if (*it != 0) { nonzero.push_back(i); }
    }
    return nonzero;
}

/*
 * Returns the elements that are nonzero (for int vectors)
 */
vector<int> non_zero_elem_int(vector<int> arr) {
    vector<int> nonzero;
    int i = 0;
    for (vector<int>::iterator it = arr.begin(); it != arr.end(); it++, i++) {
        if (*it != 0) { nonzero.push_back(*it); }
    }
    return nonzero;
}

/*
 * Returns position of median of nonzero elements
 */
int get_center_of_mass_1D(vector<uchar> arr, double noisethreshold) {
    if (std::accumulate(arr.begin(), arr.end(), 0) == 0) {
        return 0;
    }
    vector<int> nonZero = non_zero__coord_uchar(arr);
    if (nonZero.size() < arr.size() * noisethreshold) {
        return 0;
    }
    int myret = nonZero[nonZero.size() / 2];
    return myret;
}

/*
 * Returns vector of positions of median of nonzero elements for each row
 *
 * input: binary image , which axis to iterate over, threshold to remove noise
 * output: array which contains the median of non-zero indices in each row / column
 */
vector<int> get_center_of_mass_2D(vector<vector<uchar>> arr, int myaxis, double noisethreshold) {
    //myaxis = 0 is row, myaxis = 1 is columns
    //create mat object depending on myaxis
    vector<int> myret(arr.size());
    int i = 0;
    for (vector<vector<uchar>>::iterator it = arr.begin(); it != arr.end(); it++, i++) {
        myret[i] = get_center_of_mass_1D(*it, noisethreshold);
    }
    return myret;
}

/*
 * ------------------------------------------------------------------------------------------------
 *                       Helper methods for managing games/exercises
 * ------------------------------------------------------------------------------------------------
 */

//0 means not done, 1 means done and completed, -1 means done because of a crash
int getDoneState() {
    if (appHasCrashed > 0) return -1;
    else if ( (motions_done >= motionsMap.motions.size()) && (continuousGameAnimationState==GAME_STATE_NORMAL_GAMEPLAY) ) return 1;
    else return 0;
}

// returns true if done with interactivetutorial
bool getDoneInteractiveTutorial() {
    return doneInteractiveTutorial;
}

string replaceString(string str, string findText, string replaceWith) {
    size_t index = 0;
    while (true) {
        /* Locate the substring to replace. */
        index = str.find(findText, index);
        if (index == std::string::npos) break;
        
        /* Make the replacement. */
        str.replace(index, findText.length(), replaceWith);
        
        /* Advance index forward so the next iteration doesn't pick it up as well. */
        index += replaceWith.length();
    }
    return str;
}
    

std::string getCrashErrorDescription() {
    return crashErrorDescription;
}

std::string getCrashErrorDescriptionCoded() {
    std::string str = getCrashErrorDescription();
    cout << str << endl;
    return str;
    
    //Replace certain values to hide information
    str = replaceString( str, "OpenCV", "MoTrackLib");
    str = replaceString( str, "/Users/ryerrabelli/Downloads/", "[MoTrackDir]");
    str = replaceString( str, "ryerrabelli", "user");
    str = replaceString( str, "cpp", "java");
    str = replaceString( str, ".h", ".py");
    
    //Replace all lowercase letters with the previous letter in the alphabet to code it
    //This is called a substitution cipher in cryptographyhttps://en.wikipedia.org/wiki/Substitution_cipher
    std::replace( str.begin(), str.end(), 'a', '|'); //instead of z because otherwise z will be later replaced by y. | is a placeholder for now
    std::replace( str.begin(), str.end(), 'b', 'a');
    std::replace( str.begin(), str.end(), 'c', 'b');
    std::replace( str.begin(), str.end(), 'd', 'c');
    std::replace( str.begin(), str.end(), 'e', 'd');
    std::replace( str.begin(), str.end(), 'f', 'e');
    std::replace( str.begin(), str.end(), 'g', 'f');
    std::replace( str.begin(), str.end(), 'h', 'g');
    std::replace( str.begin(), str.end(), 'i', 'h');
    std::replace( str.begin(), str.end(), 'j', 'i');
    std::replace( str.begin(), str.end(), 'k', 'j');
    std::replace( str.begin(), str.end(), 'l', 'k');
    std::replace( str.begin(), str.end(), 'm', 'l');
    std::replace( str.begin(), str.end(), 'n', 'm');
    std::replace( str.begin(), str.end(), 'o', 'n');
    std::replace( str.begin(), str.end(), 'p', 'o');
    std::replace( str.begin(), str.end(), 'q', 'p');
    std::replace( str.begin(), str.end(), 'r', 'q');
    std::replace( str.begin(), str.end(), 's', 'r');
    std::replace( str.begin(), str.end(), 't', 's');
    std::replace( str.begin(), str.end(), 'u', 't');
    std::replace( str.begin(), str.end(), 'v', 'u');
    std::replace( str.begin(), str.end(), 'w', 'v');
    std::replace( str.begin(), str.end(), 'x', 'w');
    std::replace( str.begin(), str.end(), 'y', 'x');
    std::replace( str.begin(), str.end(), 'z', 'y');
    std::replace( str.begin(), str.end(), '|', 'z');//*/

    return str;
}

int getOrientationForExerciseGame(int input_exercise_game) {
    if (input_exercise_game == DEMO_LANDSCAPE_GAME || input_exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME || input_exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME || input_exercise_game == FLEXION_DOOR_KNOCK_GAME || input_exercise_game == FLEXION_CLIFF_MINE_GAME || input_exercise_game == FLEXION_PLAIN_GAME || input_exercise_game == DEVIATION_PLAIN_GAME || input_exercise_game == PRONATION_KETCHUP_SHAKE_GAME || input_exercise_game == FIST_PADDLE_BALL_GAME) {
        return 1; //landscape (with left side of phone on bottom)
    } else return 0;
}
    
void setExerciseGameAndOrientation(int input_exercise_game, int input_orientation) {
    //All these variables below are defined and set in native-lib.h/.cpp, but needed to be reset if a new game is started
    showbackground = 0;
    game_score = 0.0;
    updateCup = true;
    infoOutput = std::to_string((int) (game_score+0.5));
    feedback = "";
    percentToFill = 0.0;
    motions_done = 0;
    currentBalloonSize = BALLOON_STARTING_SIZE;
    currentCliffParam = CLIFF_STARTING_PARAM;
    currentCatapultParam = CATAPULT_STARTING_PARAM;
    catapultGameCracksDrawn = 0;
    currentKetchupParam = KETCHUP_STARTING_PARAM;
    ketchupParamIncreaseAmt = 0;
    continuousGameAnimationState = GAME_STATE_NORMAL_GAMEPLAY;
    appHasCrashed = 0;
    crashErrorDescription = "";

    //both exercise_game and orientation_game are defined in native-lib.h
    exercise_game = (int) input_exercise_game;
    orientation_game = (int) input_orientation;
}

int doSkip() {
    if (exercise_game<=0) { //this is for the "Try Background" testing phase
        motionsMap.numSkipsTot++;
        motionsMap.motions[currentMotion].numSkips++;
        return 0;
    }
    
    if ( (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD)) {
        playAudioFileClick();
        goToRedBoxStage();
        
    } else if (currentStage == STAGE_REDBOX1) {
        playAudioFileClick();
        goToGameplayStage();
        
    } else if (currentStage == STAGE_GAMEPLAY) {
        playAudioFileClick();
        motionsMap.numSkipsTot++;
        motionsMap.motions[currentMotion].numSkips++;
        if (motionsMap.motions[currentMotion].numSkips
            + motionsMap.motions[currentMotion].numRepsCompleted >=
            motionsMap.motions[currentMotion].numRepsToBeCompleted) {
            //currentMotion++;
            motions_done++; //done with this motion
            return 1;
        } else {
            incrementCurrentMotion();
        }
        
        //NOTE: paramHistory is cleared upon moving to next rep/skipping for continuous games, but is never cleared for holding games
        if (isContinuousGame(exercise_game)) {
            paramHistory.clear();
            paramHistoryTimes.clear();
        }
        
        // Temporary fix to make skip reset the balloon graphics.
        // Ideally will create a list of things that are done for each game.
        // Plus, this won't be needed in the long run since stopping the fist will automatically go to the next balloon.
        currentBalloonSize = BALLOON_STARTING_SIZE;
        currentCliffParam = CLIFF_STARTING_PARAM;
        currentCatapultParam = CATAPULT_STARTING_PARAM;
        catapultGameCracksDrawn = 0;
        currentKetchupParam = KETCHUP_STARTING_PARAM;
        ketchupParamIncreaseAmt = 0;

    }

    return 0;
}
    
bool isContinuousGame(int exercise_game) {
    return exercise_game == FIST_PUMP_BALLOON_GAME || exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME || exercise_game == FLEXION_CLIFF_MINE_GAME || exercise_game == PRONATION_KETCHUP_SHAKE_GAME;
}
    
void reportException(std::string descript) {
    appHasCrashed++;
    crashErrorDescription = descript;
}
    
void setDisplayExtraInfo(int new_displayExtraInfoAmt) {
    displayExtraInfoAmount = new_displayExtraInfoAmt;
}
    
void initializeMotionsMap(int nrtbc_each, json json_data) {
    /*
    auto var = json_data.at("game");
    auto var2 = var.at("exerciseGameNum");
    //auto var3 = json_data.at("game/exerciseGameNum");
    auto var3 = json_data.at("game").at("exerciseGameNum");
    auto mystr = json_data.at("game").at("longGameName").get<std::string>();

    auto exercise = json_data.at("exercise");
    auto rom1 = json_data.at("exercise").at("rom");*/


    motionsMap = createMap();
    // not the best approach, later pass all this info in, but for now, can just determine game-by-game
    if (exercise_game == -1) {
        //should never come here, in theory
        
    } else if (exercise_game == DEMO_PORTRAIT_GAME || exercise_game == DEMO_LANDSCAPE_GAME) { //demo exercises
        //nothing set so do nothing
        motionsMap.addMotion(0,0,nrtbc_each,0,"Testing", 0);
        
    } else if (exercise_game == FIST_PLAIN_GAME) { //for fist game plain
        motionsMap.addMotion(0,0,nrtbc_each,0,"Open your fist", 0.25);
        motionsMap.addMotion(1,0,nrtbc_each,0,"Close your fist", -0.05);
        
    } else if (exercise_game == FIST_SQUEEZE_BOTTLE_GAME) { //for fist game 1 (squeeze bottle game)
        motionsMap.addMotion(0,0,nrtbc_each,0,"Open your fist", 0.25);
        motionsMap.addMotion(1,0,nrtbc_each,0,"Close your fist", -0.05);
        
    } else if (exercise_game == FIST_PUMP_BALLOON_GAME) { //for fist game 2 (balloon game)
        motionsMap.addMotion(0,0,nrtbc_each,0,"Open and close your fist", 0);
        
    } else if (exercise_game == FIST_PADDLE_BALL_GAME) { //for fist game plain
        motionsMap.addMotion(0,0,nrtbc_each,0,"Open and close your fist", 0);
    } else if (exercise_game == HOOK_FIST_PLAIN_GAME) {
        //motionsMap.addMotion(0,0,nrtbc_each,0, "Make a hook fist", 0.25);
        //motionsMap.addMotion(1,0,nrtbc_each,0, "Make a table top", -0.25);
        motionsMap.addMotion(0, 0, nrtbc_each, 0, "Do Table Top",  1.1, COMPARE_GREATER_THAN);
        motionsMap.addMotion(1, 0, nrtbc_each, 0, "Do Hook Fist", 0.9, COMPARE_LESS_THAN);
        
    } else if (exercise_game == HOOK_FIST_CATERPILLAR_GAME) {
        //motionsMap.addMotion(0,0,nrtbc_each,0, "Make a hook fist", 0.25);
        //motionsMap.addMotion(1,0,nrtbc_each,0, "Make a table top", -0.25);
        motionsMap.addMotion(0, 0, nrtbc_each, 0, "Make a 'table top'",  1.1, COMPARE_GREATER_THAN);
        motionsMap.addMotion(1, 0, nrtbc_each, 0, "Make a 'hook fist'", 0.9, COMPARE_LESS_THAN);
        
    } else if (exercise_game == DEVIATION_PLAIN_GAME) { //for plain ulnar
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your hand leftwards", -10); // ulnar deviation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your hand rightwards", 10); // radial deviation
        } else {  // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your hand rightwards", 10); // ulnar deviation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your hand leftwards", -10); // radial deviation
        }
        
    } else if (exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME) { //for windshield game
         if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your hand leftwards", -10); // ulnar deviation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your hand rightwards", 10); // radial deviation
        } else {  // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your hand rightwards", 10); // ulnar deviation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your hand leftwards", -10); // radial deviation
        }
        
    } else if (exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME) { //for catapult game
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0,0,nrtbc_each,0, "Rotate your wrist left and right", 0);
        } else {  // default is right
            motionsMap.addMotion(0,0,nrtbc_each,0, "Rotate your wrist left and right", 0);
        }
        
    } else if (exercise_game == FLEXION_DOOR_KNOCK_GAME) { //for flex exten knock game
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your wrist upwards", -10); // "Palmar Flexion"
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your wrist downwards", 10); // "Dorsal Flexion"
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your wrist downwards", 10); // "Palmar Flexion"
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your wrist upwards", -10); // "Dorsal Flexion"
        }
        
    } else if (exercise_game == FLEXION_CLIFF_MINE_GAME) { //for cliff game
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your wrist up and down", 0);
        } else {  // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your wrist up and down", 0);
        }
        
    } else if (exercise_game == FLEXION_PLAIN_GAME) { //for flex exten basic
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your wrist upwards", -10); // "Palmar Flexion"
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your wrist downwards", 10); // "Dorsal Flexion"
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Bend your wrist downwards", 10); // "Palmar Flexion"
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Bend your wrist upwards", -10); // "Dorsal Flexion"
        }
        
    } else if (exercise_game == PRONATION_PLAIN_GAME) {
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Flip your;palm face down", -0.25); // Pronation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Flip your;palm face up", 0.25); // supination
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Flip your;palm face down",  0.25); // Pronation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Flip your;palm face up", -0.25); // Supination
        }
        
    } else if (exercise_game == PRONATION_TURN_DIAL_GAME) {
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Flip your;palm face down", -0.25); // Pronation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Flip your; palm face up ", 0.25); // supination
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Flip your;palm face down",  0.25); // Pronation
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Flip your; palm face up ", -0.25); // Supination
        }
        
    }  else if (exercise_game == PRONATION_KETCHUP_SHAKE_GAME) {
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Flip your palm;up and down", 0);
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Flip your palm;up and down",  0);
        }
        
    }  else if (exercise_game == EXERCISE_GAME_ABDUCTION_PLAIN) { //165 is for finger abduction
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Spread your fingers", 0.20); //abduction
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your fingers", -0.04); //adduction
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Spread your fingers", 0.20); //abduction
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your fingers", -0.04); //adduction
        }
        
    }  else if (exercise_game == FABDUCTION_PEACOCK_GAME) {
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Spread your fingers", 0.20); //abduction
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your fingers", -0.04); //adduction
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Spread your fingers", 0.20); //abduction
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your fingers", -0.04); //adduction
        }
        
    }  else if (exercise_game == EXERCISE_GAME_OPPOSITION_PLAIN) { //175 is for thumb oposition
        if (whichHand == Hand::LEFT) {
            //motionsMap.addMotion(0, 0, nrtbc_each, 0, "Open your thumb", -0.25);
            //motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your thumb", 0.25);
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Open your thumb",  90, COMPARE_GREATER_THAN);
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your thumb", 5, COMPARE_LESS_THAN);
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Open your thumb",  90, COMPARE_GREATER_THAN);
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your thumb", 5, COMPARE_LESS_THAN);
        }
        
    }  else if (exercise_game == TOPPOSITION_ALLIGATOR_GAME) {
        if (whichHand == Hand::LEFT) {
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Open your thumb",  90, COMPARE_GREATER_THAN);
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your thumb", 5, COMPARE_LESS_THAN);
        } else { // default is right
            motionsMap.addMotion(0, 0, nrtbc_each, 0, "Open your thumb",  90, COMPARE_GREATER_THAN);
            motionsMap.addMotion(1, 0, nrtbc_each, 0, "Close your thumb", 5, COMPARE_LESS_THAN);
        }
        
    }  else {
        //for development and testing
        
    }

    if (motionsMap.motions.size() == 2) {
        int lowerInd = 0;
        if (motionsMap.motions[0].param > motionsMap.motions[1].param) {
            lowerInd = 1;
        }
        
        auto j_flatten = json_data.flatten();

        //auto valueLow = json_data.at("exercise").at("rom").at("value_low");
        //auto valueLowDouble = json_data.at("exercise").at("rom").at("value_low").get<double>();
        if (j_flatten.find("/exercise/rom/value_low") != j_flatten.end()) {
            auto valueLow = json_data.at("exercise").at("rom").at("value_low").get<double>();
            motionsMap.motions[lowerInd].param = valueLow;
            motionsMap.motions[lowerInd].comparisonMethod = COMPARE_LESS_THAN;
        }
        if (j_flatten.find("/exercise/rom/value_high") != j_flatten.end()) {
            auto valueHigh = json_data.at("exercise").at("rom").at("value_high").get<double>();
            motionsMap.motions[(lowerInd+1) % 2].param = valueHigh;
            motionsMap.motions[(lowerInd+1) % 2].comparisonMethod = COMPARE_GREATER_THAN;
        }
        
    }

    currentMotion = 0;
    
    colorMode = json_data.at("colorMode").get<int>();
    segmentationFrameDelayAmt = json_data.at("segmentationFrameDelayAmt").get<int>();
}


void initialize(int nrtbc_each, int hand, std::string json_data_str) { // hand is 0 for right, 1 for left
    initialize(nrtbc_each, hand, json_data_str, "", "");
}

void initialize(int nrtbc_each, int hand, std::string json_data_str, std::string pbFilePath, std::string pbTxtFilePath) { // hand is 0 for right, 1 for left
    startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //try {
        whichHand = static_cast<Hand>(hand); //this needs to be set before initializeMotionsMap(.)
        
        //json json_data = ( json_data_str == "" ? "" : json::parse(json_data_str) );
        
        //The third argument being false means that an exception isn't thrown if it can't be parsed. Instead the value is just value_t::discarded.
        json json_data = json::parse(json_data_str, nullptr, false);
        if (json_data == nlohmann::detail::value_t::discarded) {
            json_data = {};
        }
        
        initializeCalibration(json_data, pbFilePath, pbTxtFilePath);
        initializeMotionsMap(nrtbc_each, json_data);
        /*
    } catch (cv::Exception ex) { //cv::Exception is a subclass of std::exception, but for some reason, if you catch it as a std::exception, calling ex.what() only returns "std::exception" not a full description
        //Exception reference: https://stackoverflow.com/questions/8415032/catching-exceptions-thrown-from-native-code-running-on-android
        reportException( ex.what() );
        
    } catch (nlohmann::json::exception ex) { //Can happen if you try to receive an element not there
        reportException( ex.what() );
        
    } catch (std::exception ex) { //cv::Exception and nlohmann::json::exception are both examples of std::exception, but this works as more of a catch-all if there is an std::exception that isn't the above categories. Once again, can't just catch this exception because then ex.what() isn't as detailed.
        reportException( ex.what() );
        
    }*/
    // For Embedded tutorial
    interactivetutorial_stage = 0;
    // this will set interactivetutorial_exercises and interactivetutorial_exercise_index = 0
    setInteractiveTutorialExercise();
    interactiveTutorialStartTime = -1;
    doneInteractiveTutorial = false;
}

/*
 * Increment current motion (set to 0 if greater than motion size).
 */
void incrementCurrentMotion() {
    currentMotion++;
    if (currentMotion >= motionsMap.motions.size()) {
        currentMotion = 0;
    }
}

/*
 * Increments the motion completed, reset the values in preperation for the next motion, and determine the info to write onto the frame for the completion event
 */
void doMotionCompletion() {
    playCompletionSound();
    paramHistory.clear();
    paramHistoryTimes.clear();
    //change cup
    //must multiply by two because the cup fills up for fist close and fist open
    percentToFill = percentToFill + 1.0/(2*motionsMap.motions[currentMotion].numRepsToBeCompleted);
    if (percentToFill >= 1.0) {
        percentToFill = 1;
    }
    updateCup = true;
    //score++;
    motionsMap.numRepsCompletedTot++;
    game_score = 10*motionsMap.numRepsCompletedTot;
    infoOutput = std::to_string((int) (game_score + 0.5));
    if (infoOutput.length() < 2) { infoOutput = " " +  infoOutput; }
    
    motionsMap.motions[currentMotion].numRepsCompleted++;
    if (motionsMap.motions[currentMotion].numSkips
        + motionsMap.motions[currentMotion].numRepsCompleted
        >= motionsMap.motions[currentMotion].numRepsToBeCompleted) {
        
        motions_done++; //is done with this motion
        //Don't do this because there is no animation because it is not a continuous game
        //continuousGameAnimationState = GAME_STATE_CONTINUOUS_GAME_ANIMATION;
        
    }
    
    if (getDoneState() > 0) { //if all motions and the ending animation (continuous game only) are done
        // TODO: insert some sort of end here
        infoOutput = "Done!";
    }
    
    incrementCurrentMotion();

}

void doMotionCompletionContinuous() {
    playCompletionSound();

    //change cup
    /*percentToFill = percentToFill + 1.0/(2*motionsMap.motions[currentMotion].numRepsToBeCompleted);
    if (percentToFill >= 1.0) {
        percentToFill = 1;
    }
    updateCup = true;*/
    
    //NOTE: paramHistory is cleared upon moving to next rep/skipping for continuous games, but is never cleared for holding games
    paramHistory.clear();
    paramHistoryTimes.clear();
    
    //score++;
    motionsMap.numRepsCompletedTot++;
    if (exercise_game == FIST_PUMP_BALLOON_GAME) {
        game_score += calculate_score_increase_for_balloon_game();
    } else if (exercise_game == FIST_PADDLE_BALL_GAME) {
        game_score += calculate_score_increase_for_paddle_game();
    } else if (exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME) {
        game_score += calculate_score_increase_for_catapult_game();
    } else if (exercise_game == FLEXION_CLIFF_MINE_GAME) {
        game_score += calculate_score_increase_for_cliff_game();
    } else if (exercise_game == PRONATION_KETCHUP_SHAKE_GAME) {
        game_score += calculate_score_increase_for_ketchup_game();
    }
    
    infoOutput = std::to_string((int)(game_score+0.5));
    if (infoOutput.length() < 2) { infoOutput = " " +  infoOutput; }


    motionsMap.motions[currentMotion].numRepsCompleted++;
    if (motionsMap.motions[currentMotion].numSkips
        + motionsMap.motions[currentMotion].numRepsCompleted
        >= motionsMap.motions[currentMotion].numRepsToBeCompleted) {
        
        motions_done++; //is done with this motion
        continuousGameAnimationState = GAME_STATE_CONTINUOUS_GAME_ANIMATION;

    }
    
    if (getDoneState() > 0) { //if all motions and the ending animation (continuous game only) are done
        // TODO: insert some sort of end here
        infoOutput = "Done!";
    }

    incrementCurrentMotion();

}

void playCompletionSound() {
    /*if (exercise_game == FIST_SQUEEZE_BOTTLE_GAME) { //for fist game 1
     playAudioFileSqueeze();
     }  else*/
    if (exercise_game == FLEXION_DOOR_KNOCK_GAME && currentMotion == 1) { //for knock game
        playAudioFileDoorknock();
        
    } else if (exercise_game == FIST_PUMP_BALLOON_GAME && currentBalloonSize == -1) { //if balloon game and balloon popped (as opposed to flew away)
        playAudioFileBalloonPop();
        
    } else if (exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME) { //catapult game
        if (currentCatapultParam == -1) { //catapult broke
            playAudioFileCatapultBreak();
        } else { //if it didn't break, the only other way the code could get to this line is if the catapult launched
            playAudioFileCatapultThrow();
        }
        
    } else if (exercise_game == FLEXION_CLIFF_MINE_GAME && currentCliffParam == -1) { //cliff game, cliff collapses
        playAudioFileCliffCollapses();
        
    } else if (exercise_game == PRONATION_KETCHUP_SHAKE_GAME) {
        if (currentKetchupParam < -0.9999 && currentKetchupParam > -1.0001) playAudioFileSpill();
        else playAudioFileEating();
        
    } else { // just play normal correct sound
        playAudioFileCorrect();//true);
    }
}


double getGameScore() {
    return game_score;
}

int getNumRepsCompletedTot() {
    return motionsMap.numRepsCompletedTot;
}


/*
 * ------------------------------------------------------------------------------------------------
 *                                  APP CORE
 *                       Controls the actual image analysis
 * ------------------------------------------------------------------------------------------------
 */
    
bool writeScoreFeedback(Mat &frame_with_mask, double param) {
    //playAudioFileCorrect(false);
    bool res;
    cv::Scalar feedbackColor = CLR_FEEDBACK_GREEN_4UC; //green
    //cout << "motion: " << currentMotion << " completed: " << checkIfParamCompleted(currentMotion, param) << " param: " << param;
    if (checkIfParamCompleted(currentMotion, param)) {
        auto time_diff =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - startTime;
        if (time_diff > holdExerciseLength) { //motion completed
            // add green tint to image
            useGreenTint = 10;
            doMotionCompletion();
        } else {
            feedback = "Hold"; // spaces added to make font size better
            //num_spaces_to_append will be equal to the difference between the smallest line ("phrase") in motionInstructions and feedback length
            int num_spaces_to_append = motionsMap.motions[currentMotion].motionInstructions.length();
            std::size_t loc_start = 0;
            while (loc_start != std::string::npos) { //npos means no matches were found
                std::size_t loc_end = motionsMap.motions[currentMotion].motionInstructions.find(';',loc_start); //will return index of space or npos if no spaces are found
                std::string phrase = motionsMap.motions[currentMotion].motionInstructions.substr(loc_start, loc_end-loc_start); //if loc_end is npos, it will go until the end of string
                if (loc_end != std::string::npos) {
                    loc_start = loc_end+1;
                } else {
                    loc_start = loc_end;
                }
                if (phrase.length() < num_spaces_to_append) num_spaces_to_append = phrase.length();
                
            }
            num_spaces_to_append = num_spaces_to_append - feedback.length();
            if (num_spaces_to_append%2 == 0) {
                string s = "";
                s.append(num_spaces_to_append/2, ' ');
                feedback = s + feedback;
                feedback.append(num_spaces_to_append/2, ' ');
            } else {
                string s = "";
                s.append(num_spaces_to_append/2, ' ');
                feedback = s + feedback;
                feedback.append(num_spaces_to_append/2+1, ' ');
            }
            //feedback = "Hold for " + std::to_string((int)(5.0-time_diff/1.0));
        }
        res = true;
    } else {
        feedback = motionsMap.motions[currentMotion].motionInstructions;//"Readjust your hand";
        feedbackColor = CLR_FEEDBACK_RED_4UC; //red
        startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        res = false;
    }
    //for putText, the last argument is thickness and must be an int. The third to last argument is the size and can be a double.
    //cv::putText(frame_with_mask,feedback, cv::Point(10,150), cv::FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,0,0,255), 2);
    //if (!badBackground) {
        /*if (stackedUpText) {
            writeText(frame_with_mask, feedback);
        } else {
            writeText(frame_with_mask, feedback, -1, int(frame_with_mask.rows * 0.20), -1.0,
                      feedbackColor);
        }

        //cv::putText(frame_with_mask,"Score: " + infoOutput, cv::Point(10,100), cv::FONT_HERSHEY_SIMPLEX, 1, Scalar(255,0,0,255), 2);

        if (stackedUpText) {
            writeText(frame_with_mask, "Score: " + infoOutput);//,-1,int(frame_with_mask.rows*0.45));
        } else {
            writeText(frame_with_mask, "Score: " + infoOutput, -1, int(frame_with_mask.rows * 0.45));
        }*/
    
    
    vector<tuple<string,cv::Scalar>> toWrite;
    if (frame_with_mask.rows > frame_with_mask.cols) { //portrait
        /*toWrite =  {
            make_tuple("Score: " + infoOutput, CLR_MOTRACK_BLUE_4UC),
            make_tuple(feedback, feedbackColor)
        };*/
        toWrite.push_back(make_tuple("  Score: "+infoOutput + "  ", CLR_MOTRACK_BLUE_4UC));
        //toWrite.push_back(make_tuple(infoOutput, CLR_MOTRACK_BLUE_4UC));
        toWrite = splitTextUpBySemicolon(toWrite, feedback, feedbackColor);
        
    } else {
        toWrite.push_back(make_tuple("  Score:"+infoOutput + "  ", CLR_MOTRACK_BLUE_4UC));
        //toWrite.push_back(make_tuple(infoOutput, CLR_MOTRACK_BLUE_4UC));
        toWrite = splitTextUpByWord(toWrite, feedback, feedbackColor);
        
    }
    

    //
    if (!badBackground) {
        writeText_onRect(frame_with_mask, toWrite);
    }
    //}
    last_param = param;
    paramHistory.push_back(param); //adds param to the end of the container (deque)
    paramHistoryTimes.push_back(chrono::steady_clock::now()); //type is chrono::steady_clock::time_point
    return res;//frame_with_mask;
}

Mat writeScoreFeedbackContinuous(Mat &frame_with_mask, double param, bool exerciseDone) {
    if (exerciseDone) {
        doMotionCompletionContinuous();
        //continuousGameResultStartTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        continuousGameResultStartTime = std::chrono::steady_clock::now();
    }

    feedback = "#" + std::to_string(motionsMap.numRepsCompletedTot+motionsMap.numSkipsTot) + "/" + std::to_string(motionsMap.numRepsToBeCompletedTot);
    infoOutput = std::to_string((int) (game_score+0.5));
    //add a lot of spaces to make text smaller in writeText for Android
    //if (frame_with_mask.cols > frame_with_mask.rows || true) { feedback   = "          " + feedback; }
    //if (frame_with_mask.cols > frame_with_mask.rows || true) { infoOutput = "          " + infoOutput; }
    
    /*if (stackedUpText) {
        writeText(frame_with_mask, feedback);
    } else {
        writeText(frame_with_mask, feedback, -1, int(frame_with_mask.rows*0.20), -1.0 );
    }

    if (stackedUpText) {
        writeText(frame_with_mask, infoOutput);//,-1,int(frame_with_mask.rows*0.45));
    } else {
        writeText(frame_with_mask, infoOutput, -1, int(frame_with_mask.rows*0.45));
    }*/
    
    vector<tuple<string,cv::Scalar>> toWrite;
    if (frame_with_mask.rows > frame_with_mask.cols) { //portrait
        toWrite =  {
            make_tuple("  Score: " + infoOutput + "  ", CLR_MOTRACK_BLUE_4UC),
            make_tuple(feedback, CLR_MOTRACK_BLUE_4UC)
        };
        
    } else {
        toWrite.push_back(make_tuple("Score:"+infoOutput, CLR_MOTRACK_BLUE_4UC));
        //toWrite.push_back(make_tuple(infoOutput, CLR_MOTRACK_BLUE_4UC));
        toWrite = splitTextUpByWord(toWrite, feedback, CLR_MOTRACK_BLUE_4UC);
    }
    
    if (!badBackground) {
        writeText_onRect(frame_with_mask, toWrite);
    }
    last_param = param;
    paramHistory.push_back(param); //adds param to the end of the container (deque)
    paramHistoryTimes.push_back(chrono::steady_clock::now()); //type is chrono::steady_clock::time_point
    return frame_with_mask;
}

bool analyzeForFistGameSqueezeBottle(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_squeeze_game(fgmask, frame_with_mask, currentMotion, ratio);
    return writeScoreFeedback(frame_with_mask,param);
}

void analyzeForFistGamePaddle(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param;
    bool hasWinner;
    std::tie(param,hasWinner) = do_paddle_ball_game(fgmask, frame_with_mask, currentMotion, ratio, motionsMap.numRepsToBeCompletedTot);
    writeScoreFeedbackContinuous(frame_with_mask,param, hasWinner);
}

void analyzeForFistPumpBalloon(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_balloon_game(fgmask, frame_with_mask, currentMotion, ratio);

    if (param > -1000.5 && param < -999.5) {
        param=0;
        writeScoreFeedbackContinuous(frame_with_mask,param, false);
        return;
    }

    double myCurrentBalloonSize, exerciseDoneNum = -1;
    double increaseParamAmt;

    if (balloon.data != NULL) {
        double startConsiderationCutoff = get_min_considering_balloon_param();
        double breakCutoff = get_max_balloon_param();
        double incrCoeff = 0.02;
        double decrCoeff = -0.004; //if you want a slight decrease when going the opposite direction (makes for better graphics)
        double stdev_lim = 0.03;
        std::tie(myCurrentBalloonSize, exerciseDoneNum, increaseParamAmt) = inferTrendFromParamHistory(param, incrCoeff, decrCoeff, stdev_lim, currentBalloonSize, breakCutoff, startConsiderationCutoff);
        currentBalloonSize = myCurrentBalloonSize;
        
    }
    
    if (currentBalloonSize < BALLOON_STARTING_SIZE && currentBalloonSize > 0) {
        currentBalloonSize = BALLOON_STARTING_SIZE;
    }

    writeScoreFeedbackContinuous(frame_with_mask,param, exerciseDoneNum > 0);
}
    
bool analyzeForFistGamePlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_plain_fist(fgmask, frame_with_mask, currentMotion, ratio);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), "  No Hand   ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "    ----    ");
    else snprintf(buf, sizeof(buf), "Param: %+0.2f", (double) param);
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));
    
    return writeScoreFeedback(frame_with_mask, param);
}
    
bool analyzeForHookFistGamePlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = analyze_hook_fist(fgmask, frame_with_mask, currentMotion, ratio);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), "  No Hand   ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "    ----    ");
    else snprintf(buf, sizeof(buf), "Param: %+0.2f", (double) param);
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));
    
    return writeScoreFeedback(frame_with_mask, param);
}
bool analyzeForHookFistCaterpillar(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_caterpillar_game(fgmask, frame_with_mask, currentMotion, ratio);
    return writeScoreFeedback(frame_with_mask,param);
}
    
    

bool analyzeForPronationPlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = analyze_pronation(fgmask,frame_with_mask,ratio);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), "  No Hand   ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "    ----    ");
    else if (param > 0) {
        snprintf(buf, sizeof(buf), "  Pro %.2f  ",   (double) param);
    } else {
        snprintf(buf, sizeof(buf), "  Sup %.2f  ", - (double) param);
    }
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));

    return writeScoreFeedback(frame_with_mask,param);
}

bool analyzeForPronationDial(Mat &fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_dial_game(fgmask, frame_with_mask, ratio);
    return writeScoreFeedback(frame_with_mask,param);
}
    
void analyzeForPronationKetchup( Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_ketchup_game(fgmask,frame_with_mask, currentMotion, ratio);
    if (displayExtraInfoAmount >= 1) {
        cout << param << endl;
    }
    
    if (param > -1000.5 && param < -999.5) {
        param=0;
        writeScoreFeedbackContinuous(frame_with_mask,param, false);
        return;
    }
    
    
    double myCurrentKetchupParam, exerciseDoneNum = -1;
    double increaseParamAmt;
    double startConsiderationCutoff = get_min_considering_ketchup_param();
    double breakCutoff = get_max_ketchup_param();
    double incrCoeff = 0.02;
    double decrCoeff = 0.02;
    double stdev_lim = 0.05;
    std::tie(myCurrentKetchupParam, exerciseDoneNum, increaseParamAmt) = inferTrendFromParamHistory(param, incrCoeff, decrCoeff, stdev_lim, currentKetchupParam, breakCutoff, startConsiderationCutoff);
    currentKetchupParam = myCurrentKetchupParam;
    ketchupParamIncreaseAmt = increaseParamAmt;
    
    if (currentKetchupParam < 0 && currentKetchupParam > -0.5) {
        currentKetchupParam = KETCHUP_STARTING_PARAM;
        ketchupParamIncreaseAmt = 0;
    }
    
    writeScoreFeedbackContinuous(frame_with_mask, param, exerciseDoneNum > 0);
}
    
bool analyzeForRadialUlnarPlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int param = bilinear_approx_analysis(fgmask,frame_with_mask, currentMotion, ratio, true);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), " No Hand  ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "   ----   ");
    else snprintf(buf, sizeof(buf), "Param: %+.0f", (double) param);
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));
    
    return writeScoreFeedback(frame_with_mask, param);
}

bool analyzeForRadialUlnarWindshield( Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int angle = do_windshield_game(fgmask,frame_with_mask, currentMotion, ratio);
    if (displayExtraInfoAmount > 1) {
        cout << angle << endl;
    }
    return writeScoreFeedback(frame_with_mask,angle);
        // if there is not enough movement don't play sound
        /*if (abs(angle-prevWindshieldAngle) == 0) {
            playAudioFileWindshield(false);
        } else {
            playAudioFileWindshield(true);
        }
        prevWindshieldAngle = angle;*/

}
    
void analyzeForRadialUlnarCatapult( Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int param = do_catapult_game(fgmask,frame_with_mask, currentMotion, ratio);
    if (displayExtraInfoAmount >= 1) {
        cout << param << endl;
    }
    
    if (param > -1000.5 && param < -999.5) {
        param=0;
        writeScoreFeedbackContinuous(frame_with_mask,param, false);
        return;
    }
    
    //There is a bug where param often reported as -180 when the algorithm incorrectly detects the bottom of the arm as the palm
    //This is a duct tape fix
    if (param < -100) {
        param = 0;
    }
    
    
    double myCurrentCatapultParam, exerciseDoneNum = -1;
    double increaseParamAmt;
    double startConsiderationCutoff = get_min_considering_catapult_param(); //+30 degrees
    double breakCutoff = get_max_catapult_param(); //was 75 degrees
    double incrCoeff = -0.002*4; //if you want a slight decrease when going the opposite direction (makes for better graphics)
    double decrCoeff = 0.01*4;
    if (whichHand==Hand::LEFT) {
        double temp = incrCoeff;
        incrCoeff = decrCoeff;
        decrCoeff = temp;
    }
    double stdev_lim = 5; //1.2
    std::tie(myCurrentCatapultParam, exerciseDoneNum, increaseParamAmt) = inferTrendFromParamHistory(param, incrCoeff, decrCoeff, stdev_lim, currentCatapultParam, breakCutoff, startConsiderationCutoff);
    currentCatapultParam = myCurrentCatapultParam;
    
    if (currentCatapultParam < 0 && currentCatapultParam > -0.5) {
        currentCatapultParam = CATAPULT_STARTING_PARAM;
    }
    
    writeScoreFeedbackContinuous(frame_with_mask, param, exerciseDoneNum > 0);
}

bool analyzeForFlexExtenPlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int param = bilinear_approx_analysis(fgmask,frame_with_mask, currentMotion, ratio, true);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), " No Hand  ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "   ----   ");
    else snprintf(buf, sizeof(buf), "Param: %+.0f", (double) param);
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));
    
    return writeScoreFeedback(frame_with_mask,param);
}

bool analyzeForFlexExtenKnock(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int param = do_knock_game(fgmask,frame_with_mask, currentMotion, ratio);
    cout << param << endl;
    
    if (displayExtraInfoAmount > 0) {
        char buf[125];
        if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), "       No Hand  ");
        else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "         ----   ");
        else snprintf(buf, sizeof(buf), "      %s: %+.0f", ((param>0) == (whichHand==Hand::LEFT))?"Flexn":"Exten", abs((double) param) );
        writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.00));
    }
    
    return writeScoreFeedback(frame_with_mask,param);
}
    
void analyzeForFlexExtenCliff(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int param = do_cliff_game(fgmask,frame_with_mask, currentMotion, ratio);
    if (displayExtraInfoAmount >= 1) {
        cout << param << endl;
    }
    
    if (param > -1000.5 && param < -999.5) {
        param=0;
        writeScoreFeedbackContinuous(frame_with_mask,param, false);
        return;
    }
    
    //There is a bug where param often reported as -180 when the algorithm incorrectly detects the bottom of the arm as the palm
    //This is a duct tape fix
    if (param < -100) {
        param = 0;
    }
    
    
    double myCurrentCliffParam, exerciseDoneNum = -1;
    double increaseParamAmt;
    double startConsiderationCutoff = get_min_considering_cliff_param(); //+30 degrees
    double breakCutoff = get_max_cliff_param(); //was 75 degrees
    double incrCoeff = -0.002*4; //if you want a slight decrease when going the opposite direction (makes for better graphics)
    double decrCoeff = 0.01*4;
    double stdev_lim = 5; //1.2
    std::tie(myCurrentCliffParam, exerciseDoneNum, increaseParamAmt) = inferTrendFromParamHistory(param, incrCoeff, decrCoeff, stdev_lim, currentCliffParam, breakCutoff, startConsiderationCutoff);
    currentCliffParam = myCurrentCliffParam;
    
    if (currentCliffParam < 0 && currentCliffParam > -0.5) {
        currentCliffParam = CLIFF_STARTING_PARAM;
        catapultGameCracksDrawn = 0;
    }
    
    writeScoreFeedbackContinuous(frame_with_mask, param, exerciseDoneNum > 0);
}
    
bool analyzeForAbductionPlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    //double param = analyze_fabduction_unrotated(fgmask,frame_with_mask, currentMotion, ratio);
    double param = analyze_fabduction(fgmask,frame_with_mask, currentMotion, ratio);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), "  No Hand   ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "    ----    ");
    else snprintf(buf, sizeof(buf), "Param: %+0.2f", (double) param);
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));
    
    return writeScoreFeedback(frame_with_mask,param);
}
    
bool analyzeForFabductionPeacock(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    double param = do_peacock_game(fgmask, frame_with_mask, currentMotion, ratio);
    return writeScoreFeedback(frame_with_mask,param);
}
    

bool analyzeForToppositionPlain(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    Mat fgmaskLarge;
    cv::resize(fgmask, fgmaskLarge, frame_with_mask.size());
    frame_with_mask.setTo(CLR_WHITE_4UC, ~fgmaskLarge);
    int param = analyze_thumb_opposition(fgmask,frame_with_mask, currentMotion, ratio);
    char buf[125];
    if      (  isParamNoHand(param)) snprintf(buf, sizeof(buf), " No Hand  ");
    else if (isParamNotValid(param)) snprintf(buf, sizeof(buf), "   ----   ");
    else snprintf(buf, sizeof(buf), "Param: %+.0f", (double) param);
    writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.50));
    
    return writeScoreFeedback(frame_with_mask,param);
}
 
    
bool analyzeForToppositionAlligator(Mat& fgmask, Mat &frame_with_mask, double ratio) {
    int angle = do_alligator_game(fgmask, frame_with_mask, currentMotion, ratio);
    return writeScoreFeedback(frame_with_mask,angle);
}
    
    

bool analyzeDefault( Mat& fgmask, Mat &frame_with_mask, double ratio) {
    return true;
}

/*
 * Removes or dims the background
 */
void removeBackground(Mat& frameDisplay, const Mat& fgmask, Mat &frame_with_mask) {
    if (showbackground == 3) { //dims background very strongly
        Mat bgmask;
        bitwise_not(fgmask,bgmask);
        Mat foreground = Mat::zeros(frameDisplay.size(), frameDisplay.type());
        Mat background = Mat::zeros(frameDisplay.size(), frameDisplay.type());
        bitwise_and(frameDisplay, frameDisplay, foreground, fgmask); //output into foreground
        bitwise_and(frameDisplay, frameDisplay, background, bgmask); //alternatively use Scalar::all(255)-fgmask instead of bgmask

        double alpha = 1; //increase contrast if this is greater than 1
        double beta = -127*(alpha-1); //-127*(alpha-1) is enough to offset contrast, anymore/less will cause saturation
        foreground.convertTo(foreground, -1, alpha, beta); // new_image = alpha*image + beta; used to increase contrast/saturate
        addWeighted(foreground,1.0,background,0.25,0,frame_with_mask); //output into frame_with_mask
        // for text

        //cv::rectangle(frame_with_mask, cv::Rect(0,0,500,500), cv::Scalar( 250, 250, 250,1), -1, 8);
    } else if (showbackground == 2) { //removes the background (not just dims it)
        //frame_with_mask.setTo(cv::Scalar(127,127,127,255));
        bitwise_and(frameDisplay, frameDisplay, frame_with_mask, fgmask);
        //frame_with_mask.setTo(cv::Scalar(127,127,127,255));
        //frame_with_mask.setTo(cv::Scalar(127,127,127,255), fgmask);

    } else if (showbackground == 1) { //used to called "preprocessdim". This dims out the background.
        Mat bgmask;
        bitwise_not(fgmask,bgmask);
        Mat foreground = Mat::zeros(frameDisplay.size(), frameDisplay.type());
        Mat background = Mat::zeros(frameDisplay.size(), frameDisplay.type());
        bitwise_and(frameDisplay, frameDisplay, foreground, fgmask); //output into foreground
        bitwise_and(frameDisplay, frameDisplay, background, bgmask); //alternatively use Scalar::all(255)-fgmask instead of bgmask
        addWeighted(foreground,1.0,background,0.5,0,frame_with_mask); //output into  frame_with_mask
    }
}

// Analyzes hand images and displays game
int analyzeAndDisplay(Mat& frameEdit, Mat& frameDisplay, Mat& fgmask, Mat &frame_with_mask, double ratio) {
    if (exercise_game == -1) {
        //should never come here, in theory
        cout << "Error, exercise game is -1";
        return 0;
        
    } else if (exercise_game == DEMO_PORTRAIT_GAME || exercise_game == DEMO_LANDSCAPE_GAME) { //nothing set so do nothing on the screen besides the background subtraction
        cv::resize(fgmask, fgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, fgmask, frame_with_mask);
        return 0;
        
    } else if (exercise_game == FIST_PLAIN_GAME) { //for fist game plain
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if (analyzeForFistGamePlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == FIST_SQUEEZE_BOTTLE_GAME) { //for fist game 1 (squeeze bottle)
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        if (analyzeForFistGameSqueezeBottle(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == HOOK_FIST_PLAIN_GAME) { //for hook fist game plain
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if (analyzeForHookFistGamePlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == HOOK_FIST_CATERPILLAR_GAME) { //for hook fist game plain
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        if (analyzeForHookFistCaterpillar(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == FIST_PUMP_BALLOON_GAME) { //for fist game 2 (pump balloon)
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        analyzeForFistPumpBalloon(fgmask, frameDisplay, ratio);
        return 0; // continuous games have white outline
        
    } else if (exercise_game == FIST_PADDLE_BALL_GAME) { //for fist game 3 (paddle ball)
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        analyzeForFistGamePaddle(fgmask, frameDisplay, ratio);
        return 0; // continuous games have white outline

    } else if (exercise_game == DEVIATION_PLAIN_GAME) { //for plain ulnar
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if (analyzeForRadialUlnarPlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME) { //for windshield game
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        if (analyzeForRadialUlnarWindshield(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
    } else if (exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME) { //for catapult game
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        analyzeForRadialUlnarCatapult(fgmask, frameDisplay, ratio);
        return 0;
    } else if (exercise_game == FLEXION_DOOR_KNOCK_GAME) { //for flex exten knock game
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameEdit.clone();
        if (analyzeForFlexExtenKnock(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }

    } else if (exercise_game == FLEXION_CLIFF_MINE_GAME) { //for cliff game
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        analyzeForFlexExtenCliff(fgmask, frameDisplay, ratio);
        return 0;
        
    } else if (exercise_game == FLEXION_PLAIN_GAME) { //for flex exten plain
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if (analyzeForFlexExtenPlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == PRONATION_PLAIN_GAME) {
        //189 193 181
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if ( analyzeForPronationPlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == PRONATION_TURN_DIAL_GAME) {
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameEdit.clone();
        if ( analyzeForPronationDial(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == PRONATION_KETCHUP_SHAKE_GAME) {
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameEdit.clone();
        analyzeForPronationKetchup(fgmask, frameDisplay, ratio);
        return 0;
        
    } else if (exercise_game == EXERCISE_GAME_ABDUCTION_PLAIN) {
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if (analyzeForAbductionPlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == FABDUCTION_PEACOCK_GAME) {
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        if (analyzeForFabductionPeacock(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == EXERCISE_GAME_OPPOSITION_PLAIN) {
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if (analyzeForToppositionPlain(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else if (exercise_game == TOPPOSITION_ALLIGATOR_GAME) {
        //Do not remove background. Background will instead be replace with an image, and this is done  inside the analyze method.
        frame_with_mask = frameDisplay.clone();
        if (analyzeForToppositionAlligator(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
        
    } else { //for development and testing
        Mat resizedFgmask;
        cv::resize(fgmask, resizedFgmask, frameDisplay.size(),0,0, INTER_NEAREST );
        removeBackground(frameDisplay, resizedFgmask, frame_with_mask); //frame_with_mask);
        frameDisplay = frame_with_mask;
        if  (analyzeDefault(fgmask, frameDisplay, ratio)) {
            return 1;
        } else {
            return -1;
        }
    }
}


#ifdef EXTERN_C
}
#endif
