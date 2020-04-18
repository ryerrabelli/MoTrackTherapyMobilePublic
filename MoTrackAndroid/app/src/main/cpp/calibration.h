//
// Created by benka on 11/25/18.
//

#ifndef MOTRACKTHERAPYMOBILE_CALIBRATION_H
#define MOTRACKTHERAPYMOBILE_CALIBRATION_H


//Below are necessary for use in pre-processor directive IF statements which are used for device-specific code
#define ANDROID_OS 1
#define IOS -1
#define MAC_OS -2
#define LINUX_OS -3
//if this were Android, CMakeList would have already defined MY_OS to be +1. Otherwise, this must be iOS so define it here to be -1
#ifndef MY_OS
    #define MY_OS IOS //MY_OS represents this device. Set to iOS if not defined already.
#endif

//Need to have opencv2 included here so CV_MAJOR_VERSION is defined
#include <opencv2/opencv.hpp>

#if CV_MAJOR_VERSION >= 4
    //OpenCV 4 will throw an error for functions returning Mat objects if the file is encoded with extern "C", saying "XYZ has C-linkage specified, but returns user-defined type ABC which is incompatible with C"
    #undef EXTERN_C //undefining it is like setting it to "false"
#else
    #define EXTERN_C //define EXTERN_C as in put
#endif

#define CLR_MOTRACK_BLUE_4UC    cv::Scalar( 48,171,223,255) //30ABDF
#define CLR_MOTRACK_BLUE_3UC    cv::Scalar( 48,171,223)     //30ABDF
#define CLR_MOTRACK_GRAY_4UC    cv::Scalar(242,242,242,255) //#F2F2F2 (value found by copying from themeroller on Feb 22, 2019)
#define CLR_MOTRACK_GRAY_3UC    cv::Scalar(242,242,242)     //#F2F2F2
#define CLR_BLACK_4UC           cv::Scalar(  0,  0,  0,255) //#000000
#define CLR_BLACK_3UC           cv::Scalar(  0,  0,  0)     //#000000
#define CLR_WHITE_4UC           cv::Scalar(255,255,255,255)
#define CLR_WHITE_3UC           cv::Scalar(255,255,255)
#define CLR_FEEDBACK_GREEN_4UC  cv::Scalar(  0,255,  0,255)
#define CLR_FEEDBACK_RED_4UC    cv::Scalar(255,  0,  0,255)
#define CLR_DISABILITY_RED_4UC  cv::Scalar(255,  0,  0,255)

#define STAGE_GAMEPLAY 0
#define STAGE_BG_SUBTRACT_STANDARD 1
#define STAGE_BG_SUBTRACT_BEGINNING 2
#define STAGE_REDBOX1 3
#define STAGE_INTERACTIVETUTORIAL 4
#define STAGE_NOT_SET -1

/*
 * Interactive tutorial stages
 */

// STAGE
#define INTERACTIVETUTORIAL_GENERAL_INTRO 0
#define INTERACTIVETUTORIAL_EXERCISE_INTRO 1
#define INTERACTIVETUTORIAL_GAME_INTRO 2
// WHICH EXERCISE
#define INTERACTIVETUTORIAL_OPENFIST 11
#define INTERACTIVETUTORIAL_CLOSEFIST 12
#define INTERACTIVETUTORIAL_ULNARDEV 21
#define INTERACTIVETUTORIAL_RADIALDEV 22
#define INTERACTIVETUTORIAL_WRISTFLEX 31
#define INTERACTIVETUTORIAL_WRISTEXTEN 32
#define INTERACTIVETUTORIAL_SUPINATION 41
#define INTERACTIVETUTORIAL_PRONATION 42
#define INTERACTIVETUTORIAL_FINGERABD 51
#define INTERACTIVETUTORIAL_FINGERADD 52
#define INTERACTIVETUTORIAL_HOOKFIST 61
#define INTERACTIVETUTORIAL_TABLETOP 62
#define INTERACTIVETUTORIAL_THUMBOPPO 71
#define INTERACTIVETUTORIAL_THUMBREPO 72
/*
 * End of interactive tutorial stages
 */

#define DEMO_PORTRAIT_GAME 0
#define DEMO_LANDSCAPE_GAME 5
#define FIST_SQUEEZE_BOTTLE_GAME 100
#define FIST_PUMP_BALLOON_GAME 102
#define FIST_PADDLE_BALL_GAME 104
#define FIST_PLAIN_GAME 105
#define HOOK_FIST_CATERPILLAR_GAME 110
#define HOOK_FIST_PLAIN_GAME 115
#define DEVIATION_WINDSHIELD_WIPER_GAME 120
#define DEVIATION_CATAPULT_LAUNCH_GAME 122
#define DEVIATION_PLAIN_GAME 125
#define FLEXION_DOOR_KNOCK_GAME 130
#define FLEXION_CLIFF_MINE_GAME 132
#define FLEXION_PLAIN_GAME 135
#define PRONATION_TURN_DIAL_GAME 150
#define PRONATION_KETCHUP_SHAKE_GAME 152
#define PRONATION_PLAIN_GAME 155
#define FABDUCTION_PEACOCK_GAME 160
#define EXERCISE_GAME_ABDUCTION_PLAIN 165
#define TOPPOSITION_ALLIGATOR_GAME 170
#define EXERCISE_GAME_OPPOSITION_PLAIN 175

#define GAME_STATE_CONTINUOUS_GAME_ANIMATION 1
#define GAME_STATE_NORMAL_GAMEPLAY 0

#define PARAM_NOT_VALID -100000
#define PARAM_NO_HAND -100001


#include <time.h>
#include <chrono>
#include "motion_info.h"
#include "FaceDetector.h"
#include "json.hpp"


#ifdef EXTERN_C
extern "C" {
#endif

struct distInfo {
    cv::Mat dist;
    cv::Point maxLoc;
    // Replaced from double to float on Feb 5, 2019 hoping to make the algorithm faster.
    // Untested if it worked.
    std::vector<float> xdata;
    std::vector<float> ydata;
};


/*
* Interactive Tutorial Variables
*/
// Initialize a Map of int & int[] using initializer_list
extern std::map<int, std::vector<int>> mapOfInteractiveExercises;

extern int interactivetutorial_stage;
extern std::vector<int> interactivetutorial_exercises;
extern int interactivetutorial_exercise_index;
extern std::time_t interactiveTutorialStartTime;

extern bool doneInteractiveTutorial;
extern double holdExerciseLength;
extern std::time_t startTime;
/*
 * End of interactive tutorial variables
 */


extern cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg;
extern cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_h;
extern cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_s;
extern cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_v;
extern cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_mov;
extern double learning_rate;
extern cv::Mat calibrationKern;
extern int useGreenTint;
extern bool stackedUpText;
extern int displayExtraInfoAmount;
extern int fontType;
extern double last_param;
extern std::deque<double> paramHistory; //NOTE: paramHistory is cleared upon moving to next rep/skipping for continuous games, but is never cleared for holding games
extern std::deque<std::chrono::steady_clock::time_point> paramHistoryTimes;
//extern std::deque<std::chrono::time_point<std::chrono::microseconds>> paramHistoryTimes;
extern struct MotionsMap motionsMap;

//extern std::time_t continuousGameResultStartTime;
extern std::chrono::steady_clock::time_point continuousGameResultStartTime;
extern int continuousGameAnimationState;

extern int showbackground;

extern int  inputImageWidth;
extern int  inputImageHeight;
extern bool inputImageDimChanged; //changed from last frame?
extern bool shouldWriteOnIm;
extern std::vector<std::tuple<std::string,double, cv::Point, cv::Scalar>> bufferForWriting; // buffer only valid if above is false, will wait to write at end

extern vector<double> tapLocationXs;
extern vector<double> tapLocationYs;
extern vector<bool> tapLocationsProcessed;
extern vector<std::chrono::steady_clock::time_point> tapStartTimes;
    
extern cv::Mat sizeRefImg;
extern cv::Mat sizeRefImgOrig;
extern cv::Mat sizeRefImgAlpha;

extern std::vector<int> fgLocAlgOrder;
extern int fgLocAlgInd;
extern int fgLocAlg;
extern double bgOnlyOTSUThresh;
extern cv::Rect textWriteRectangle;
extern cv::Scalar textWriteRectangleBackground;
// if true, that means background is bad and needs to be reset
extern bool badBackground;
// text for when above is true
extern std::string firstline_badBackground;
extern std::string secondline_badBackground;
extern vector<tuple<string,cv::Scalar>> badBackground_toWrite;
// this is which hand the user is doing exercises for
enum Hand { RIGHT,LEFT};
extern Hand whichHand;
    
extern FaceDetector faceDetector;
    
extern int currentStage;


extern int exercise_game;
    
extern int colorMode;
extern int segmentationFrameDelayAmt;

extern cv::dnn::Net net;


void addHandForSizeReference(cv::Mat& frameDisplay, bool isPortrait, int speedReq, bool isTutorial);

//pbFilePath and pbTxtFilePath don't matter for non-ML algorithms
void initializeCalibration(nlohmann::json json_data, std::string pbFilePath, std::string pbTxtFilePath);

void setLearningRate(bool learn);

// This flipLearningRate function is called by objective c++ file OpenCVWrapper in the
// iOS version of the app unlike the android version which called a function in native-lib
// first, which then directs to the setLearningRate function above, the iOS version goes
// directly to calibration
bool flipLearningRate();

int timedBackgroundRemoval(cv::Mat im, bool, bool);

void goToRedBoxStage();
void goToGameplayStage();
void resetCalibration();
    
/*
 * Takes in integer representing which motion we're on, and
 * the double representing that performance, and returns if doing well
 */
bool checkIfParamCompleted(int mot, double p);

distInfo get_distance_transform(cv::Mat fgmask);
distInfo get_distance_transform_adj(Mat fgmask, bool adjusted_max);

void checkBackground(cv::Mat& fgmask, cv::Mat& frameDisplay, double ratio);
std::tuple<double, double> circularMeanStdDev(Mat angles, int min, int max);
void preprocess(cv::Mat& frameEdit, cv::Mat &frameDisplay, cv::Mat &fgmask, double ratio, int drawColor = 0);

void ensureHandAtTopLevel(cv::Mat& frameDisplay, cv::Mat& fgmask, cv::Mat& originalIm);
void colorizeSegmentation(const Mat &score, Mat &segm);
double otsu_8u_with_mask(const cv::Mat1b src, const cv::Mat1b& mask);
double threshold_with_mask(cv::Mat1b& src, cv::Mat1b& dst, double thresh, double maxval, int type, const cv::Mat1b& mask = cv::Mat1b());
std::tuple<double, double> otsu_8u_with_mask_returning_separation(const cv::Mat1b src, const cv::Mat1b& mask);
double threshold_mod_OTSU(cv::Mat1b& src, cv::Mat1b& dst, const cv::Mat1b& mask);

void getLargestContour(cv::Mat& frameDisplay, cv::Mat& fgmask, double ratio, int drawColor = 0, bool drawConvexHull = false);
cv::Mat getAllLargeContours(cv::Mat& frameDisplay, cv::Mat fgmask, double min_size, double ratio, int drawColor = 0); //min_size should be a decimal percentage between 0 and 1 inclusive (although near 1 wouldn't make any sense) or -1 indicating default
cv::Mat getAllLargeContours2(Mat& frameDisplay, Mat fgmask, double min_size, double ratio, int drawColor, cv::Mat kernelSmall, cv::Mat kernelBig);
cv::Mat getAllLargeContours3(Mat& frameDisplay, Mat fgmask, double min_size, double ratio, int drawColor, cv::Mat kernelSmall, cv::Mat kernelBig);

    
//for some reason, referring to Size instead of cv::Size results in an ambiguity compilation error in Xcode
int getPropThickness(cv::Size frameDisplaySize, int standardThickness);

void writeBadBackgroundText(cv::Mat& frameDisplay);
int getThicknessFromFontScale(double fontScale);
double getFontScaleFromThickness(double thickness);
    
void applyColorMapKeepingAlpha(cv::Mat& picture, int map);
    
//need to the fourth value in color, not just cv::Scalar(255,0,0) because otherwise, it will be the color of the imageView background in iOS
double writeText(cv::Mat& img, std::string s,  int xloc = -1, int yloc = -1, double fontScale = -1.0, cv::Scalar color = cv::Scalar(255,0,0,255));//  (1,1, cv::CV_8U, 0));
double writeText_onRect(cv::Mat& img, std::vector<std::tuple<std::string,cv::Scalar>> ss);

//static const int* getFontData(int fontFace);

//double getTextScaleFromHeight(int fontFace, int thickness, int height);

//0 degrees is rightward, 90 degrees is downward (increasing y), had to make them different names because for some reason it was causing a compile error in Android
void drawDashedLine(cv::Mat &drawOn, cv::Point startPt, cv::Point endPt, double dashPercentage, int dashCt, const Scalar& color, int thickness);
void drawDashes(cv::Mat &drawOn, cv::Point startPt, double degrees, double dashLength, double spaceLength, int dashCt, const Scalar &color, int thickness);

cv::Mat copyWithPixelTransparencyUsingAlpha(cv::Mat& background, cv::Mat& foreground, double start_x, double start_y, double width, double height, int a);

//Below is a significantly (up to 20x) faster algorithm to copyWithPixelTransparencyUsingAlpha that does the same thing
cv::Mat drawImageOptimized(cv::Mat &background, cv::Mat &foreground, int start_x, int start_y);

    
//Statistical analysis of continuous games
std::tuple<double, double, double> inferTrendFromParamHistory(double param, double incrCoeff, double decrCoeff, double stdev_lim, double currentBalloonSize, double breakCutoff, double startConsiderationCutoff);
    
vector<tuple<string,cv::Scalar>> splitTextUpByWord(vector<tuple<string,cv::Scalar>> toWrite, string inputText, Scalar color);
vector<tuple<string,cv::Scalar>> splitTextUpBySemicolon(vector<tuple<string,cv::Scalar>> toWrite, string inputText, Scalar color);

    
bool isParamNoHand(double param);
bool isParamNotValid(double param);
bool isParamNoExerciseDone(double param);
std::string paramToStr(double param);
double subtractParams(double paramEst, double paramLabel);

#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_CALIBRATION_H
