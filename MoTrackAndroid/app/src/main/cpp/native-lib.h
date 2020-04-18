//
//  native-lib.h
//  MoTrackTestiOS/MoTrack Android
//
//  Created by Rahul Yerrabelli on 12/27/18 / 11/24/18
//  Copyright © 2018 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_NATIVE_LIB_H
#define MOTRACKTHERAPYMOBILE_NATIVE_LIB_H


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
#include <opencv2/opencv.hpp>
#include "motion_info.h"
#include <string>
#include "json.hpp"



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

// Get Methods
int
Java_com_motracktherapy_motrack_CVActivity_getCurrentMotion(JNIEnv *env, jobject);
int
Java_com_motracktherapy_motrack_CVActivity_getSkips(JNIEnv *env, jobject);
int
Java_com_motracktherapy_motrack_CVActivity_getNumCompleted(JNIEnv *env, jobject, jint mot);
double
Java_com_motracktherapy_motrack_CVActivity_getGameScore(JNIEnv *env, jobject);
int
Java_com_motracktherapy_motrack_CVActivity_getNumRepsCompletedTot(JNIEnv *env, jobject);
jstring
Java_com_motracktherapy_motrack_CVActivity_getDescription(JNIEnv *env, jobject);
int
Java_com_motracktherapy_motrack_CVActivity_getMotionTitle(JNIEnv *env, jobject);
//0 means not done, 1 means done and completed, -1 means done because of a crash
int
Java_com_motracktherapy_motrack_CVActivity_getDoneState(JNIEnv *env, jobject);
bool
Java_com_motracktherapy_motrack_CVActivity_getDoneInteractiveTutorial(JNIEnv *env, jobject);
jstring
Java_com_motracktherapy_motrack_CVActivity_getCrashErrorDescription(JNIEnv *env, jobject);
jstring
Java_com_motracktherapy_motrack_CVActivity_getCrashErrorDescriptionCoded(JNIEnv *env, jobject);
    
void
Java_com_motracktherapy_motrack_CVActivity_setCurveArrows(JNIEnv *env, jobject, long addrLeft,
                                                          long addrRight, jboolean jincludes_alpha);
    
/*
 * Initializes variables
 */
void Java_com_motracktherapy_motrack_CVActivity_initialize(
        JNIEnv *env,
        jobject, /* this */
        jint jnrtbc_each, jint hand, jstring json_data);

/*
 * Toggles learning rate
 */
void
Java_com_motracktherapy_motrack_CVActivity_setLearning(JNIEnv *env, jobject, jboolean jlearn);

/*
 * Set's the amount of extra info to display. This is used for better testing.
 */
void
Java_com_motracktherapy_motrack_CVActivity_setDisplayExtraInfo(JNIEnv *env, jobject, jint jdisplayExtraInfoAmount);

/*
 * Increments skip
 * @return 1 if currentMotion was incremented, 0 otherwise
 */
int
Java_com_motracktherapy_motrack_CVActivity_doSkip(JNIEnv *env, jobject, jboolean jlearn);

    
/*
 * Toggles show background
 */
void
Java_com_motracktherapy_motrack_CVActivity_setShowingBackground(JNIEnv *env, jobject, jint jshow);

/*
 * Tells java whether game should be landscape or portrait depending on the game
 */
int
Java_com_motracktherapy_motrack_CVActivity_getOrientationForExerciseGame(JNIEnv *env, jobject, jint jexercise_game);
    
/*
 * Sets which game/exercise to do and which orientatiion the screen should be in
 */
void
Java_com_motracktherapy_motrack_CVActivity_setExerciseGameAndOrientation(JNIEnv *env, jobject,
                                                                         jint jexercise_game,
                                                                         jint jorientation);

/*
 * Takes image, preprocesses, analyzes, and displays
 */
jboolean
Java_com_motracktherapy_motrack_CVActivity_inputImage(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight, jobject srcBuffer,
        jobject dstSurface, jboolean, jboolean);


void Java_com_motracktherapy_motrack_CVActivity_resetCalibrationBackground( JNIEnv *env, jobject obj);
/*
 * Takes image, preprocesses, analyzes, and displays
 */
void
Java_com_motracktherapy_motrack_CVActivity_redoImage(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight,
        jobject dstSurface);

/*
 * This method is for testing purposes.  It will display the camera without alteration
 */
void
Java_com_motracktherapy_motrack_CVActivity_displayCamera(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight, jobject srcBuffer,
        jobject dstSurface);

// Not technically a "Java_com_..." function but included because it requires JNIEnv *env access
// and is specific to android
// This function's purpose is to set feedback text view to given text
void setFeedbackTextFromC(JNIEnv *env, jobject obj, std::string s);

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */

    int cppInt(int input);
    
    int shouldCameraBeInitializedAsContinuous(int input); 

    void setSizeRefImg(cv::Mat picture);

    void initializeLightsTutorial(string json_data_str);
    bool getLightsTutorialDoneState();
    cv::Mat doLightsTutorial(cv::Mat frameDisplay, int returnWidth, int returnHeight);

    void registerTap(double x, double y, int displayWidth, int displayHeight);
    
    std::tuple<cv::Mat,cv::Mat> inputImage(cv::Mat input, int returnWidth, int returnHeight, bool isTesting, bool doInteractiveTutorial);

    /*void removeBackground(cv::Mat flipRgba, cv::Mat fgmask, cv::Mat &frame_with_mask);

     bool analyzeAndDisplay(cv::Mat flipRgba, cv::Mat fgmask, cv::Mat &frame_with_mask) ;*/

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

extern int orientation_game; // -5 is invalid, 0 for portrait, 1 for landscape (with phone left on bottom)
extern int motions_done; //0 means none are done; this variable was previously named done_state

extern int currentMotion;
extern cv::Mat mYuvlast;
extern double game_score;
extern std::string infoOutput;
extern std::string feedback;
extern int prevWindshieldAngle;
extern int outlineColor;
extern cv::Mat sizeRefImgOrig;
extern cv::Mat sizeRefImg;
extern cv::Mat greenTint;
//extern auto startTime;

extern int appHasCrashed;
extern std::string crashErrorDescription;




/*
 * ------------------------------------------------------------------------------------------------
 *                       Helper methods used in image analysis & image modification
 * ------------------------------------------------------------------------------------------------
 */

// Takes in input cv::Mat with channels rgba, color value for red, blue, and green
// Sets all pixels with that color value to transparent
cv::Mat setPixelTransparent(cv::Mat &input_rgba, int r, int g, int b);

void resetCalibrationBackground();

//Controls automatic calibration for the first few seconds after opening a camera game
int runTimedCalibration(bool, cv::Mat, bool, bool);


/*
 * Converts cv::Mat object to 2D std::vector
 */
std::vector<std::vector<uchar>> toVec(const cv::Mat MatIn);

/*
 * Returns positions of nonzero elements (for uchar std::vectors)
 */
std::vector<int> non_zero__coord_uchar(std::vector<uchar> arr);

/*
 * Returns positions of nonzero elements (for int std::vectors)
 */
std::vector<int> non_zero_coord_int(std::vector<int> arr);

/*
 * Returns the elements that are nonzero (for int std::vectors)
 */
std::vector<int> non_zero_elem_int(std::vector<int> arr);

/*
 * Returns position of median of nonzero elements
 */
int get_center_of_mass_1D(std::vector<uchar> arr, double noisethreshold);

/*
 * Returns std::vector of positions of median of nonzero elements for each row
 *
 * input: binary image , which axis to iterate over, threshold to remove noise
 * output: array which contains the median of non-zero indices in each row / column
 */
std::vector<int> get_center_of_mass_2D(std::vector<std::vector<uchar>> arr, int myaxis, double noisethreshold);



/*
 * ------------------------------------------------------------------------------------------------
 *                       Helper methods for managing games/exercises
 * ------------------------------------------------------------------------------------------------
 */

bool getDoneInteractiveTutorial();

int getDoneState();

std::string getCrashErrorDescription();
std::string getCrashErrorDescriptionCoded();

int getOrientationForExerciseGame(int input_exercise_game);
    
void setExerciseGameAndOrientation(int input_exercise_game, int input_orientation);

int doSkip();
    
bool isContinuousGame(int exercise_game);
    
void setDisplayExtraInfo(int new_displayExtraInfoAmt);
    
void reportException(std::string descript);

void initializeMotionsMap(int nrtbc_each, nlohmann::json json_data);

void initialize(int nrtbc_each, int hand, std::string json_data_str); // hand is 0 for right, 1 for left
void initialize(int nrtbc_each, int hand, std::string json_data_str, std::string pbFilePath, std::string pbTxtFilePath); // hand is 0 for right, 1 for left

/*
 * Increment current motion (set to 0 if greater than motion size).
 */
void incrementCurrentMotion();

/*
 * Increments the motion completed, reset the values in preperation for the next motion, and determine the info to write onto the frame for the completion event
 */
void doMotionCompletion();

void doMotionCompletionContinuous();
    
void playCompletionSound();


double getGameScore();

int getNumRepsCompletedTot();


/*
 * ------------------------------------------------------------------------------------------------
 *                                  APP CORE
 *                       Controls the actual image analysis
 * ------------------------------------------------------------------------------------------------
 */

bool writeScoreFeedback(cv::Mat &frame_with_mask, double param);

cv::Mat writeScoreFeedbackContinuous(cv::Mat &frame_with_mask, double param, bool exerciseDone);

bool analyzeForFistGamePlain(Mat& fgmask, Mat &frame_with_mask, double ratio);
bool analyzeForFistGameSqueezeBottle(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
void analyzeForFistPumpBalloon( cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
void analyzeForFistGamePaddle(Mat& fgmask, Mat &frame_with_mask, double ratio);
    
bool analyzeForHookFistGamePlain(Mat& fgmask, Mat &frame_with_mask, double ratio);
bool analyzeForHookFistCaterpillar(Mat& fgmask, Mat &frame_with_mask, double ratio);

bool analyzeForRadialUlnarPlain( cv::Mat& fgmask, cv::Mat &frame_with_mask, double);
    
bool analyzeForFlexExtenPlain(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
bool analyzeForFlexExtenKnock(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
void analyzeForFlexExtenCliff(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
    
bool analyzeForAbductionPlain(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
    
bool analyzeForToppositionPlain(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);
    
bool analyzeDefault(cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);

void removeBackground(cv::Mat& flipRgba, const cv::Mat& fgmask, cv::Mat &frame_with_mask);

// Analyzes hand images and displays game
int analyzeAndDisplay(cv::Mat& frameEdit, cv::Mat& frameDisplay, cv::Mat& fgmask, cv::Mat &frame_with_mask, double ratio);

#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_NATIVE_LIB_H
