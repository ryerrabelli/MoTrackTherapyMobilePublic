//
// Created by benka on 11/25/18.
//
//The calibration import must always be the first import in order to make use of device-dependent preprocessor directives
#include "calibration.h"

//device specific includes/imports
#if MY_OS==ANDROID_OS
//Below are the imports for the android version
#include <android/log.h>

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS) && 1<0
//Below are the imports for the iOS version
#include <opencv2/ximgproc.hpp>

#endif



// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <opencv2/opencv.hpp>
#if MY_OS==LINUX_OS
    #include <opencv2/tracking.hpp>
#endif
#include <math.h>
#include <cmath>
#include <time.h>
#include <chrono>
#include <opencv2/dnn.hpp>

//#include "all_access_vars.h"
#include "audio_player.h"
#include "motion_info.h"

#include "json.hpp"

//#include "FaceDetector.h"


using namespace cv;
using namespace std;
using json = nlohmann::json;


#ifdef EXTERN_C
extern "C" {
#endif

/*
* Interactive Tutorial Variables
*/
// Initialize a Map of int & int[] using initializer_list
std::map<int, std::vector<int>> mapOfInteractiveExercises = {
        {DEMO_PORTRAIT_GAME, {}},
        {DEMO_LANDSCAPE_GAME, {}},
        {FIST_SQUEEZE_BOTTLE_GAME, {INTERACTIVETUTORIAL_OPENFIST, INTERACTIVETUTORIAL_CLOSEFIST}},
        {FIST_PUMP_BALLOON_GAME, {INTERACTIVETUTORIAL_OPENFIST, INTERACTIVETUTORIAL_CLOSEFIST}},
        {FIST_PADDLE_BALL_GAME, {INTERACTIVETUTORIAL_OPENFIST, INTERACTIVETUTORIAL_CLOSEFIST}},
        {FIST_PLAIN_GAME, {INTERACTIVETUTORIAL_OPENFIST, INTERACTIVETUTORIAL_CLOSEFIST}},
        {HOOK_FIST_CATERPILLAR_GAME, {INTERACTIVETUTORIAL_HOOKFIST, INTERACTIVETUTORIAL_TABLETOP}},
        {HOOK_FIST_PLAIN_GAME, {INTERACTIVETUTORIAL_HOOKFIST, INTERACTIVETUTORIAL_TABLETOP}},
        {DEVIATION_WINDSHIELD_WIPER_GAME, {INTERACTIVETUTORIAL_ULNARDEV, INTERACTIVETUTORIAL_RADIALDEV}},
        {DEVIATION_CATAPULT_LAUNCH_GAME, {INTERACTIVETUTORIAL_ULNARDEV, INTERACTIVETUTORIAL_RADIALDEV}},
        {DEVIATION_PLAIN_GAME, {INTERACTIVETUTORIAL_ULNARDEV, INTERACTIVETUTORIAL_RADIALDEV}},
        {FLEXION_DOOR_KNOCK_GAME, {INTERACTIVETUTORIAL_WRISTFLEX, INTERACTIVETUTORIAL_WRISTEXTEN}},
        {FLEXION_CLIFF_MINE_GAME, {INTERACTIVETUTORIAL_WRISTFLEX, INTERACTIVETUTORIAL_WRISTEXTEN}},
        {FLEXION_PLAIN_GAME, {INTERACTIVETUTORIAL_WRISTFLEX, INTERACTIVETUTORIAL_WRISTEXTEN}},
        {PRONATION_TURN_DIAL_GAME, {INTERACTIVETUTORIAL_PRONATION, INTERACTIVETUTORIAL_SUPINATION}},
        {PRONATION_KETCHUP_SHAKE_GAME, {INTERACTIVETUTORIAL_PRONATION, INTERACTIVETUTORIAL_SUPINATION}},
        {PRONATION_PLAIN_GAME, {INTERACTIVETUTORIAL_PRONATION, INTERACTIVETUTORIAL_SUPINATION}},
        {FABDUCTION_PEACOCK_GAME, {INTERACTIVETUTORIAL_FINGERADD, INTERACTIVETUTORIAL_FINGERABD}},
        {EXERCISE_GAME_ABDUCTION_PLAIN, {INTERACTIVETUTORIAL_FINGERADD, INTERACTIVETUTORIAL_FINGERABD}},
        {TOPPOSITION_ALLIGATOR_GAME, {INTERACTIVETUTORIAL_THUMBREPO, INTERACTIVETUTORIAL_THUMBOPPO}},
        {EXERCISE_GAME_OPPOSITION_PLAIN, {INTERACTIVETUTORIAL_THUMBREPO, INTERACTIVETUTORIAL_THUMBOPPO}},
};

int interactivetutorial_stage = 0;
std::vector<int> interactivetutorial_exercises;
int interactivetutorial_exercise_index = 0;
std::time_t interactiveTutorialStartTime = -1;

bool doneInteractiveTutorial = false;
double holdExerciseLength= 1.0;
std::time_t startTime = -1;
/*
 * End of interactive tutorial variables
 */


cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg;

cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_h;
cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_s;
cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_v;

cv::Ptr<cv::BackgroundSubtractorMOG2> fgbg_mov;

bool displayWhiteOutline = false;

bool prev25 = false; //value will be flipped upon initialize calibration
double bgOnlyOTSUThresh = -1; //used for OTSU thresholding method (alg 30) to detect foreground, is the thresh value when hand isn't in the frame (bg only)
cv::Scalar colorLowerBound, colorUpperBound;
std::vector<cv::Scalar> colorLowerBounds, colorUpperBounds;
cv::Scalar colorMean, colorStDev;
cv::Mat backgroundExample;
cv::Mat foregroundExample;

double lowerBoundH = 0,upperBoundH = 180;
CascadeClassifier faceCascadeClassifier;

Mat roi_hist_meanShift;
Rect2d trackWindow;
bool faceCascadeClassifierLoaded = false;

#if MY_OS==LINUX_OS
//Ptr<Tracker> mot_tracker = Tracker::create( "KCF" );
Ptr<Tracker> mot_tracker;
#endif

double learning_rate = 0.2;
//NOTE: kernelStandardWd, kernelStandardHt are used in multiple places, not just here. Be careful when changing the values.
int kernelStandardWd = 5, kernelStandardHt = 5;
cv::Mat calibrationKern = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelStandardWd, kernelStandardHt));
cv::Mat calibrationKernBigger;
std::time_t calibStartTime;
int useGreenTint = 0;
// The last y position text is written on. Used for lining up the text.
int last_y;
// if stackedUpText is false, negates Ben's stuff to put text at the top. Necessary on Rahul's phone since screen is cutoff
bool stackedUpText = false;
// 0 is false/none
// 1 is the normal value before implementation of this int (which is minimal drawings for guidance, okay for user use, but not ideal)
// 2+ is for testing/debugging
int displayExtraInfoAmount = 0;
int fontType = cv::FONT_HERSHEY_DUPLEX;
double last_param = 0;
std::deque<double> paramHistory; //NOTE: paramHistory is cleared upon moving to next rep/skipping for continuous games, but is never cleared for holding games
std::deque<std::chrono::steady_clock::time_point> paramHistoryTimes;
struct MotionsMap motionsMap;

//std::time_t continuousGameResultStartTime;
std::chrono::steady_clock::time_point continuousGameResultStartTime = std::chrono::steady_clock::now();
int continuousGameAnimationState = GAME_STATE_NORMAL_GAMEPLAY; //0 means normal gameplay or not a continuous game; 1 means doing the ending animation of a continuous game

int showbackground = 0; //explanation of showbackground values are defined in removeBackground(.) function

// if true, that means background is bad and needs to be reset
bool badBackground = false;
// text for when above is true
std::string firstline_badBackground =  "   Background lost   ";
std::string secondline_badBackground = "Hit the reset button.";
vector<tuple<string,cv::Scalar>> badBackground_toWrite;

cv::Rect textWriteRectangle = cv::Rect(0,0,0,0);
cv::Scalar textWriteRectangleBackground = CLR_MOTRACK_GRAY_4UC;//cv::Scalar( 250, 250, 250,100);
int  inputImageWidth = 0;
int  inputImageHeight = 0;
bool inputImageDimChanged = false; //changed from last frame?
bool shouldWriteOnIm = true; // if true, that means have space on top of image in ios
std::vector<std::tuple<std::string, double,cv::Point,cv::Scalar>> bufferForWriting; // buffer only valid if above is false, will wait to write at end

vector<double> tapLocationXs = {};
vector<double> tapLocationYs = {};
vector<bool> tapLocationsProcessed = {};
vector<std::chrono::steady_clock::time_point> tapStartTimes;

cv::Mat sizeRefImg;
cv::Mat sizeRefImgOrig;
cv::Mat sizeRefImgAlpha;

//fgLocAlgOrder is if (shouldWriteOnIm) the order of algorithms that will be cycled through every time "reset" button is clicked, looping back as necessary

//**************** FGLOCALGORDER IS OVER HERE ****************
std::vector<int> fgLocAlgOrder = {1684};//{25};//{21,23,31,29,24,0};//{29,28,25,26,27,20,1,0,}; //0 is MOG2 original algorithm; 42 is the MOG2 + OTSU
int fgLocAlgInd = 0; //index within fgLocAlgOrder of the algorithm for background subtraction (value here doesn't matter bc set in initializeCalibration anyway)
int fgLocAlg = -1; //current algorithm for background subtraction


Hand whichHand;

int currentStage = -1;

int exercise_game = -1;

int colorMode = -1;
int segmentationFrameDelayAmt = 1;

    /*
    class ContinuousGame {
        // Access specifier
        public:

        // Data Members
        std::string geekname;

        // Member Functions()
        void printname()
        {
            cout << "Geekname is: " << geekname;
        }

        ContinuousGame()
        {
            std::cout << "D\n";
        }

    };

    class BalloonGame : public ContinuousGame {
        // Access specifier
        public:

        // Data Members
        //std::string geekname;

        // Member Functions()
        void printname()
        {
            cout << "Geekname is: " << geekname;
        }
    };

    class CatapultGame : public ContinuousGame {
        // Access specifier
        public:

        // Data Members
        //std::string geekname;

        // Member Functions()
        void printname()
        {
            cout << "Geekname is: " << geekname;
        }
    };*/


cv::dnn::Net net;

//used in iOS only (for now at least)
std::string pbFilePath = "";
std::string pbTxtFilePath = "";

int frameCounter = 0;

//pbFilePath and pbTxtFilePath don't matter for non-ML algorithms
void initializeCalibration(json json_data, std::string thisPbFilePath, std::string thisPbTxtFilePath) {
    //used in iOS only (for now at least)
    pbFilePath    = thisPbFilePath;
    pbTxtFilePath = thisPbTxtFilePath;

    fgbg = createBackgroundSubtractorMOG2(500,16,false);
    setLearningRate(true);


    //fgLocAlgOrder = std::vector<int>();
    //fgLocAlgOrder = {json_data};
    fgLocAlgOrder = json_data["segmentationAlg"].get<std::vector<int>>();

    fgLocAlgInd = -1; //will be incremented in resetCalibration
    resetCalibration();

    //make this proportional to image size eventually
    calibrationKernBigger = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelStandardWd*3, kernelStandardHt*3));

    /*badBackground_toWrite =  {
            make_tuple(firstline_badBackground, cv::Scalar(255,0,0)),
            make_tuple(secondline_badBackground, cv::Scalar(255,0,0))
    };*/
    std::string text = "Background lost    ;Hit the reset button";
    badBackground_toWrite = splitTextUpByWord(badBackground_toWrite, text, cv::Scalar(255,0,0));

    textWriteRectangle = cv::Rect(0,0,0,0);

    frameCounter = 0;
}

void resetCalibration() {
    calibStartTime = 0.0;
    currentStage = STAGE_BG_SUBTRACT_BEGINNING; //must set it here because this is used in preprocess before being set in runTimedCalibration. It also avoids a bug where pressing back during redbox period makes it start in redbox period next time for this aforementioned reason (calibStartTime becomes negative from preprocess before the value of calibStartTime 0 can be used).

    //The following is in order to test different types of background subtraction
    fgLocAlg = fgLocAlgOrder[++fgLocAlgInd % fgLocAlgOrder.size()]; //increase fgLocAlgInd and get the next fgLoc in the list, looping through if necessary
    if (displayExtraInfoAmount == -1) {
        fgLocAlg = 1674;
    } else if (displayExtraInfoAmount == -2) {
        fgLocAlg = 1670;
    } else if (displayExtraInfoAmount == -3) {
        fgLocAlg = 1684;
    } else if (displayExtraInfoAmount == -4) {
        fgLocAlg = 1680;
    }

    if (fgLocAlg == 21 || fgLocAlg == 4) {
        fgbg_h = createBackgroundSubtractorMOG2(500,16,false);
    }
    if (fgLocAlg == 6 || fgLocAlg == 8 || fgLocAlg == 28 || fgLocAlg == 29 || fgLocAlg == 1600 || fgLocAlg == 1604 || fgLocAlg == 1610 || fgLocAlg == 1614) {
        fgbg_h = createBackgroundSubtractorMOG2(500,16,false);
        fgbg_s = createBackgroundSubtractorMOG2(500,16,false);
        fgbg_v = createBackgroundSubtractorMOG2(500,16,false);
    }
    if (fgLocAlg == 31 || fgLocAlg == 1550 || fgLocAlg == 1551) {
        fgbg_mov = createBackgroundSubtractorMOG2(500,16,false);
    }
    if (fgLocAlg >= 3000 && fgLocAlg < 5000) {
        //Example: OpenCV's samples/dnn/segmentation.cpp, URL: https://docs.opencv.org/master/d4/d88/samples_2dnn_2segmentation_8cpp-example.html
        std::string dnnModelPrefix; //NOT used for iOS
        std::string model, config;
        #if MY_OS==ANDROID_OS
            dnnModelPrefix = "/storage/emulated/0/Download/"; //Doesn't work (even file is in internal storage on Ben's phone)
            model  = dnnModelPrefix+"tensorflowModel.pb";
            config = dnnModelPrefix+"tensorflowModel.pbtxt";

        #elif MY_OS==IOS
            //dnnModelPrefix not used for iOS
            model = pbFilePath;
            config = pbTxtFilePath;
            net = cv::dnn::readNetFromTensorflow(model);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);


        #elif (MY_OS==MAC_OS || MY_OS==LINUX_OS)
            dnnModelPrefix = "../../MoTrackAndroid/app/src/main/assets/dnnmodels/";
            //technically inputted model and config should be type cv::String not std::string, but std::String can be implicitly converted to cv::String
            // "path to the .pb file with binary protobuf description of the network architecture"
            //std::string model  = "dnnmodels/frozen_graph.pb";
            model  = dnnModelPrefix+"tensorflowModel.pb";//"frozen_inference_graph.pb";
            // "path to the .pbtxt file that contains text graph definition in protobuf format..."
            //std::string config = "";
            config = dnnModelPrefix+"tensorflowModel.pbtxt";//"ssd_mobilenet_v2_coco_2018_03_29.pbtxt.txt";

            net = cv::dnn::readNetFromTensorflow(model);
            //net.setPreferableBackend(backendId);
            //net.setPreferableTarget(DNN_TARGET_OPENCL);
            //net.setPreferableTarget(targetId);
        #endif


    }

    prev25 = !prev25;

    badBackground = false;

    colorLowerBounds.clear();
    colorUpperBounds.clear();

    tapLocationXs.clear();
    tapLocationYs.clear();
    tapLocationsProcessed.clear();
    tapStartTimes.clear();
}

void goToRedBoxStage() {
    calibStartTime = -6; //set calib start time to 6 seconds ago
    currentStage = STAGE_REDBOX1;
}

void goToGameplayStage() {
    calibStartTime = -11; //set calib start time to 11 seconds ago
    currentStage = STAGE_GAMEPLAY;
}

void setLearningRate(bool learn) {
    if (learn) {
        learning_rate = 0.2;
    } else {
        learning_rate = 0.0;
    }
}

// This flipLearningRate function is called by objective c++ file OpenCVWrapper in the
// iOS version of the app unlike the android version which called a function in native-lib
// first, which then directs to the setLearningRate function above, the iOS version goes
// directly to calibration
bool flipLearningRate() {
    if (learning_rate > 0.001 ||  learning_rate < 0.000000001) {
        learning_rate = 0;
        return false;
    } else {
        learning_rate = 0.2;
        return true;
    }
}



// goes through process of removing background automatically (on a timer)
// returns true when done
int timedBackgroundRemoval(Mat frameDisplay, bool restart, bool portrait_intended) {
    if (restart) calibStartTime = 0.0;
    if (calibStartTime <= 0.0)  {
        //if calibStartTime is 0, make it current time. If negative, make it that much time ago
        calibStartTime += std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    auto time_diff =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())-calibStartTime;

    if (time_diff >= 5.0) {
        setLearningRate(false);
        //calibStartTime = 0.0;
        return (int) time_diff;
    } else {
        setLearningRate(true);
        //if ((exercise_game>0 || motionsMap.numSkipsTot%2==0)) {
        if (fgLocAlg<3000 && (exercise_game>0 || motionsMap.numSkipsTot%2==0)) { //Don't do this for AI-based algorithms, which are fgLocAlg above 3000
            if (portrait_intended) {
                std::string s1 = "Remove head ";
                string s2 = "& hand from";
                std::string s3 = "view for ";
                string s4 = std::to_string((int) (5.0 - time_diff / 1.0)) + " secs";

                //std::string s1 = "Remove head and hand from view ";
                //std::string s2 = "for " + std::to_string((int)(5.0-time_diff/1.0)) + " seconds";
                //cv::putText(frameDisplay, s, cv::Point(10, 150), cv::FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0, 255), 2);
                if (stackedUpText) {
                    double fontSize = writeText(frameDisplay, s1);
                    writeText(frameDisplay, s2, -1, -1, fontSize);
                    writeText(frameDisplay, s3, -1, -1, fontSize);
                    writeText(frameDisplay, s4, -1, -1, fontSize);
                } else {
                    double fontSize = writeText(frameDisplay, s1, -1, int(frameDisplay.cols * 0.20));
                    Size textSize = cv::getTextSize(s1,cv::FONT_HERSHEY_SIMPLEX,fontSize, 5, 0);
                    writeText(frameDisplay, s2, -1, int(frameDisplay.cols * 0.20) + 1.5*textSize.height, fontSize);
                    writeText(frameDisplay, s3, -1, int(frameDisplay.cols * 0.20) + 3.0*textSize.height, fontSize);
                    writeText(frameDisplay, s4, -1, int(frameDisplay.cols * 0.20) + 4.5*textSize.height, fontSize);
                }
            } else {
                std::string s1 = "Remove head & hand";
                std::string s2 =
                "from view for " + std::to_string((int) (5.0 - time_diff / 1.0)) + " secs";
                //std::string s1 = "Remove head and hand from view ";
                //std::string s2 = "for " + std::to_string((int)(5.0-time_diff/1.0)) + " seconds";
                //cv::putText(frameDisplay, s, cv::Point(10, 150), cv::FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0, 255), 2);
                if (stackedUpText) {
                    double fontSize = writeText(frameDisplay, s1);
                    writeText(frameDisplay, s2, -1, -1, fontSize);
                } else {
                    double fontSize = writeText(frameDisplay, s1, -1, int(frameDisplay.cols * 0.20));
                    writeText(frameDisplay, s2, -1, int(frameDisplay.cols * 0.35), fontSize);
                }
            }
        }

        return int(time_diff);
    }
}

/*
 * Takes in integer representing which motion we're on, and
 * the double representing that performance, and returns if doing well
 */
bool checkIfParamCompleted(int mot, double p) {
    if (isParamNoExerciseDone(p)) return false;

    //if comparisonMethod is COMPARE_LESS_THAN, then p must be less than the set param
    //if comparisonMethod is COMPARE_GREATER_THAN, then p must be greater than the set param
    //else p must be further away from 0 than the set param (unless param is 0, in which case p must be positive)
    if ( (motionsMap.motions[mot].param < 0 && motionsMap.motions[mot].comparisonMethod != COMPARE_GREATER_THAN) || motionsMap.motions[mot].comparisonMethod == COMPARE_LESS_THAN) {
        return (p < motionsMap.motions[mot].param);
    } else {
        return (p > motionsMap.motions[mot].param);
    }
}

distInfo get_distance_transform(Mat fgmask) {
    // Perform the distance transform algorithm
    Mat dist_transform;
    cv::distanceTransform(fgmask, dist_transform, DIST_L2, 3);
    // using L2 seem to get the most consistent result
    cv::GaussianBlur(dist_transform, dist_transform, Size(5, 5), 0, 0);
    // get the maximum intensity location of dist
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point maxLocPoint;

    cv::minMaxLoc(dist_transform, &minVal, &maxVal, &minLoc, &maxLocPoint);
    distInfo d;
    d.dist = dist_transform;
    d.maxLoc = maxLocPoint;
    return d;

}

distInfo get_distance_transform_adj(Mat fgmask, bool adjusted_max) {
    // Perform the distance transform algorithm
    Mat dist_transform;
    cv::distanceTransform(fgmask, dist_transform, DIST_L2, 3);
    // using L2 seem to get the most consistent result
    cv::GaussianBlur(dist_transform, dist_transform, Size(5, 5), 0, 0);
    // get the maximum intensity location of dist
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point maxLocPoint;

    distInfo d;
    if (!adjusted_max) {
        cv::minMaxLoc(dist_transform, &minVal, &maxVal, &minLoc, &maxLocPoint);
        d.dist = dist_transform;
        d.maxLoc = maxLocPoint;
    } else {
        int i = 0;
        cv::Mat roiMask(fgmask.rows, fgmask.cols, CV_8UC1, 255);
        float totradius = roiMask.rows-1;
        while (true) {
            //cout << i++ << " " << totradius << endl;
            //cout << i++ << " ";
            cv::minMaxLoc(dist_transform, &minVal, &maxVal, &minLoc, &maxLocPoint, roiMask);
            //cout << "a";
            if (maxLocPoint.y >= totradius*0.95) { //if bottom most row
                //cout << "b";
                float radius = dist_transform.at<float>(maxLocPoint.y, maxLocPoint.x);
                //cout << "c";
                cv::circle(roiMask, maxLocPoint, (int) radius, 0, FILLED);
                //cout << "d";
                totradius = maxLocPoint.y - radius;
                //cout << "e";
                if (totradius < fgmask.rows*1.0/3) {
                    d.dist = dist_transform;
                    d.maxLoc = maxLocPoint;
                    //cout << "f";
                    break;
                }
            } else {
                d.dist = dist_transform;
                d.maxLoc = maxLocPoint;
                //cout << "g";
                if (i > 1) {
                    //cout << "!";
                }
                break;
            }
            cout << endl;
        }
    }
    return d;

}

int getPropThickness(Size frameDisplaySize, int standardThickness) {
    return (int) (standardThickness*std::max(frameDisplaySize.height, frameDisplaySize.width)/512.0+0.5);
}

// takes in background subtracted mask, returns image showing only largest contour
void getLargestContour(Mat& frameDisplay, Mat& fgmask, double ratio, int drawColor, bool drawConvexHull) {
    // if draw Color is 0, that means draw white. if -1, that means you're doing it wrong (red). if 1, then green
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(fgmask,contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
    double max_area = 0;
    int max_ct_ind = -1;
    for (int i = 0; i < contours.size(); i++) {
        double ct_area = cv::contourArea(contours[i]);
        if (ct_area > max_area) {
            max_area = ct_area;
            max_ct_ind = i;
        }
    }
    if (max_area/(fgmask.rows*fgmask.cols) < 0.05) { // prevent small objects from being detected
        vector<cv::Point> empty;
        //return empty; //cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
    }
    if (max_ct_ind != -1) {
        fgmask.setTo(0); //fgmask= cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        if (drawConvexHull) {
            vector<vector<Point>> hull(1);
            convexHull(Mat(contours[max_ct_ind]), hull[0], false);
            cv::drawContours(fgmask, hull, 0, 255, -1);
            //drawContours(frameDisplay, hull, 0, cv::Scalar(0,255,0), -1);//1, 8, vector<Vec4i>(), 0, cv::Point());
        } else {
            cv::drawContours(fgmask, contours, max_ct_ind, 255, -1);

            //Below lines remove any internal contours (i.e. if you make your hands into an O shape, this code makes sure the O isn't drawn as the foreground solely because it is surrounded by foreground)
            if (exercise_game == TOPPOSITION_ALLIGATOR_GAME || exercise_game == EXERCISE_GAME_OPPOSITION_PLAIN) {
                for (int i = 0; i < contours.size(); i++) {
                    if (i != max_ct_ind) cv::drawContours(fgmask, contours, i, 0, -1);
                }
            }

        }


        //draw an outline of the hand

        if (displayWhiteOutline || drawColor != 0 || showbackground == 0) { //if displayWhiteOutline, skip if drawColor == 0 since we decided drawing the white outline wasn't very helpful
            if (drawColor != 0 && learning_rate < 0.05 && (exercise_game>0 || motionsMap.numSkipsTot%2==0)) {
                for(int i1=0; i1<contours.size(); i1++) {
                    for(int i2=0; i2<contours[i1].size(); i2++) {
                        contours[i1][i2] = contours[i1][i2]/ratio;
                    }
                }

                //double epsilon = 0.01 * cv::arcLength( contours[max_ct_ind], true );
                //cv::approxPolyDP( contours[max_ct_ind],contours[max_ct_ind], epsilon, true );

                int thickness = getPropThickness(frameDisplay.size(),4);
                if (drawColor == 1) { // motion is correct, draw green
                    cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(0, 255, 0, 255),
                                     thickness); //color is green
                } else if (drawColor == -1){ // motion is incorrect, draw red
                    cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(255, 0, 0, 255),
                                     thickness); //color is red
                } else if (drawColor == 0) {
                    cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(255, 255, 255, 255),
                                     thickness); //color is white
                }
                //cv::polylines( frameDisplay, Mat( contours[ct_ind] ) * ratio, true, Scalar(127,255,255,255), 10); // draws contour resized 2x
                //return contours[max_ct_ind];
            }
        }

        //return cimg;
        //vector<cv::Point> empty;
        //return empty;
    }

    //vector<cv::Point> empty;
    //return empty;
    //return fgmask;
}

void drawHistogram(Mat img, bool isHue) {
    vector<Mat> channels;//Mat bgrHsv[3];
    split(img,channels);
    for (int chan = 0; chan < channels.size(); chan++) {
        int bins = 25;
        MatND hist;
        const int histSize = MAX(bins, 2);
        float range[] = {0, 255};
        if (isHue && chan==0) {
            range[1] = 180;
        }
        const float *ranges = {range};

        /// Get the Histogram and normalize it
        calcHist(&channels[chan], 1, 0, Mat(), hist, 1, &histSize, &ranges, true, false);
        normalize(hist, hist, 0, 255, NORM_MINMAX, -1, Mat());


        /// Draw the histogram
        int w = 400;
        int h = 400;
        int bin_w = cvRound((double) w / histSize);
        Mat histImg = Mat::zeros(w, h, CV_8UC3);

        for (int i = 0; i < bins; i++) { rectangle(histImg, Point(i * bin_w, h),
                                                   Point((i + 1) * bin_w, h -
                                                                          cvRound(hist.at<float>(
                                                                                  i) * h / 255.0)),
                                                   Scalar(0, 0, 255), -1);
        }

        //imshow("Histogram_" + std::to_string(chan), histImg);
    }
}

//min_size should be a decimal percentage between 0 and 1 inclusive (although near 1 wouldn't make any sense) or -1 indicating default
cv::Mat getAllLargeContoursWithKernelOperations(Mat& frameDisplay, Mat fgmask, double min_size, double ratio, int drawColor, cv::Mat kernelSmall, cv::Mat kernelBig) {
    //chrono::steady_clock::time_point t1 = chrono::steady_clock::now();

    if (min_size > -1.01 && min_size < -0.99) {
        min_size = 0.01;
    }
    fgmask.setTo(255, fgmask>0);
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    //[Next, Previous, First_Child, Parent]
    cv::findContours(fgmask,contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    double max_area = 0;
    int max_ct_ind = -1;
    vector<int> ctInds;
    for (int i = 0; i < contours.size(); i++) {
        //element 3 represents the First_Child of the contour. Since there are only two layers in a RETR_CCOMP-style hierarchy, external boundaries have no First_Child
        if (hierarchy[i][3] != -1) continue; //checks if the contour is a hole boundary, aka an internal boundary

        double ct_area = cv::contourArea(contours[i]);
        if (ct_area/(fgmask.rows*fgmask.cols) < min_size) { // prevent small objects from being detected
            //return cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        } else {
            if (ct_area > max_area) {
                max_area = ct_area;
                max_ct_ind = i;
            }
            ctInds.push_back(i);
        }
    }

    if (max_ct_ind == -1) {
        return cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));

    } else  {
        cv::Mat new_fgmask = cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        cv::Mat frameDisplayMajContours = cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        cv::Mat frameDisplayMinContours = cv::Mat(frameDisplay.rows, frameDisplay.cols, CV_8U, double(0));
        vector<vector<cv::Point> > contoursBig; //identical to contours, but the points are designed for the frameDisplay not fgmask space
        for (int i=0; i < contours.size(); i++) {
            contoursBig.push_back( {} );
            for (int j=0; j < contours[i].size(); j++) {
                contoursBig[i].push_back( contours[i][j]/ratio );
            }
        }

        for (int ind = 0; ind < ctInds.size(); ind++) {
            int this_ct_ind = ctInds[ind];
            //element 3 represents the First_Child of the contour. Since there are only two layers in a RETR_CCOMP-style hierarchy, external boundaries have no First_Child
            if (hierarchy[this_ct_ind][3] != -1) { //checks if the contour is a hole boundary, aka an internal boundary
                continue;
            }

            cv::drawContours(new_fgmask, contours, this_ct_ind, 255, -1);

            //draw an outline of the individual contours (i.e the hand)
            if (learning_rate < 0.05 && (exercise_game>0 || motionsMap.numSkipsTot%2==0)) {
                //When drawing the contours, set to fill inside instead of a border because otherwise dilation won't work properly
                //later we can convert filled inside to a border using a morphological gradient operation
                if (this_ct_ind == max_ct_ind) { //draw major contour
                    cv::drawContours(frameDisplayMajContours, contours, this_ct_ind, Scalar(255), -1);
                } else { //draw non major contours
                    cv::drawContours(frameDisplayMinContours, contours, this_ct_ind, Scalar(255), -1);
                }
            }
        }

        if (exercise_game == TOPPOSITION_ALLIGATOR_GAME || exercise_game == EXERCISE_GAME_OPPOSITION_PLAIN) {
            for (int i = 0; i < contours.size(); i++) {
                //element 3 represents the First_Child of the contour. Since there are only two layers in a RETR_CCOMP-style hierarchy, external boundaries have no First_Child
                if (hierarchy[i][3] != -1) { //checks if the contour is a hole boundary, aka an internal boundary.
                    cv::drawContours(new_fgmask, contours, i, Scalar(0), -1);
                }
            }
        }


        //Dilation operations.
        //In 1684 etc, this is done to get the outline of the fingers/hands which might not be caught because they are slightly different color
        //this makes the color lines slightly within the mask. This can't easily be avoided because if we put this line
        //inside getLargestContour before drawing the lines, you'd also have to somehow dilate the contour too
        //8-9 seconds
        cv::dilate( new_fgmask,              new_fgmask,              kernelSmall );
        cv::dilate( frameDisplayMajContours, frameDisplayMajContours, kernelSmall );
        cv::dilate( frameDisplayMinContours, frameDisplayMinContours, kernelSmall );//*/

        //4 seconds
        //convert solid area to border using a gradient operation (subtraction between dilation and erosion)
        cv::Mat kernelBorder; //kernel to make border is half of the image kernel (tested empirically to look nice)
        cv::resize(kernelSmall, kernelBorder, cv::Size(), (1.0)/2, (1.0)/2, cv::INTER_NEAREST);
        //kernelBorder = kernelSmall;
        //cv::morphologyEx(frameDisplayMajContours, frameDisplayMajContours, MORPH_GRADIENT, kernelBorder);
        //cv::morphologyEx(frameDisplayMinContours, frameDisplayMinContours, MORPH_GRADIENT, kernelBorder);
        //Instead of gradient (subtraction between dilation and erosion), make your own operation that is
        // subtraction between original and erosion. When there is only a major contour or when the major contours and minor contours are far apart, there is no visible difference because the dilation part is covered up by the background scenery image, but when the contours are near each other, it prevents the border from looking twice as large in some parts. This happened in IMG_0005_C for example
        Mat frameDisplayMajContoursEroded, frameDisplayMinContoursEroded;
        cv::erode(frameDisplayMajContours, frameDisplayMajContoursEroded, kernelBorder);
        cv::erode(frameDisplayMinContours, frameDisplayMinContoursEroded, kernelBorder);
        cv::subtract(frameDisplayMajContours, frameDisplayMajContoursEroded, frameDisplayMajContours);
        cv::subtract(frameDisplayMinContours, frameDisplayMinContoursEroded, frameDisplayMinContours);//*/

        cv::resize(frameDisplayMajContours, frameDisplayMajContours, frameDisplay.size());
        cv::resize(frameDisplayMinContours, frameDisplayMinContours, frameDisplay.size());

        //do minor, then major, because major is arguably more important and should be visible
        if (displayWhiteOutline || drawColor != 0 || showbackground == 0) {
            frameDisplay.setTo( Scalar(  0,  0,255,255),  frameDisplayMinContours ); //color is blue
            if (drawColor == 1) { // motion is correct, draw green
                //cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(0, 255, 0, 255), thickness); //color is green
                frameDisplay.setTo( Scalar(  0, 255,   0, 255), frameDisplayMajContours ); //color is green
            } else if (drawColor == -1){ // motion is incorrect, draw red
                //cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(255, 0, 0, 255), thickness); //color is red
                frameDisplay.setTo( Scalar(255,   0,   0, 255), frameDisplayMajContours ); //color is red
            } else if (drawColor == 0) {
                //cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(255, 255, 255, 255), thickness); //color is white
                frameDisplay.setTo( Scalar(255, 255, 255, 255), frameDisplayMajContours ); //color is white
            }
        }


        //chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
        //auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        //cout << time_span.count()/1000.0 << "ms" << endl; //*/

        return new_fgmask;

    }
    return fgmask;
}


//min_size should be a decimal percentage between 0 and 1 inclusive (although near 1 wouldn't make any sense) or -1 indicating default
cv::Mat getAllLargeContours(Mat& frameDisplay, Mat fgmask, double min_size, double ratio, int drawColor) {
    chrono::steady_clock::time_point t1 = chrono::steady_clock::now();

    if (min_size > -1.01 && min_size < -0.99) {
        min_size = 0.01;
    }
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(fgmask,contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    double max_area = 0;
    int max_ct_ind = -1;
    vector<int> ctInds;
    for (int i = 0; i < contours.size(); i++) {
        double ct_area = cv::contourArea(contours[i]);
        if (ct_area/(fgmask.rows*fgmask.cols) < min_size) { // prevent small objects from being detected
            //return cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        } else {
            if (ct_area > max_area) {
                max_area = ct_area;
                max_ct_ind = i;
            }
            ctInds.push_back(i);
        }
    }

    if (max_ct_ind == -1) {
        return cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));

    } else  {
        cv::Mat new_fgmask = cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        for (int ind = 0; ind < ctInds.size(); ind++) {
            cv::drawContours(new_fgmask, contours, ctInds[ind], 255, -1);

            int this_ct_ind = ctInds[ind];
            for(int i2=0; i2<contours[this_ct_ind].size(); i2++) {
                contours[this_ct_ind][i2] = contours[this_ct_ind][i2]/ratio;
            }

            //draw an outline of the individual contours (i.e the hand)
            if (displayWhiteOutline || drawColor != 0 || showbackground == 0) {
                if (learning_rate < 0.05 && (exercise_game>0 || motionsMap.numSkipsTot%2==0)) {
                    int thickness = getPropThickness(frameDisplay.size(),4);
                    if (this_ct_ind == max_ct_ind) {
                        //draw major contour
                        if (drawColor == 1) { // motion is correct, draw green
                            cv::drawContours(frameDisplay, contours, this_ct_ind, Scalar(0, 255, 0, 255), thickness); //color is green
                        } else if (drawColor == -1){ // motion is incorrect, draw red
                            cv::drawContours(frameDisplay, contours, this_ct_ind, Scalar(255, 0, 0, 255), thickness); //color is red
                        } else if (drawColor == 0) {
                            cv::drawContours(frameDisplay, contours, this_ct_ind, Scalar(255, 255, 255, 255), thickness); //color is white
                        }

                    } else {
                        //draw non major contours
                        cv::drawContours(frameDisplay, contours, this_ct_ind, Scalar(  0,  0,255,255), thickness); //color is blue

                    }

                    //cv::polylines( frameDisplay, Mat( contours[ct_ind] ) * ratio, true, Scalar(127,255,255,255), 10); // draws contour resized 2x
                }
            }
        }

        chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
        auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        cout << time_span.count()/1000.0 << "ms" << endl; //*/

        return new_fgmask;
    }
    return fgmask;
}

Mat filterOutBrightest(Mat img) {
    Mat gray;
    cv::cvtColor(img,gray,cv::COLOR_BGR2GRAY);
    cv::GaussianBlur( gray, gray, Size( 5, 5 ), 0, 0 );
    double minVal;
    double maxVal;
    cv::Point minLoc;
    cv::Point maxLocPoint;

    cv::minMaxLoc(gray, &minVal, &maxVal, &minLoc, &maxLocPoint);
    maxVal = maxVal - 10;
    for (int r = 0; r < gray.rows; r++) {
        for (int c = 0; c < gray.cols; c++) {
            if (gray.at<float>(r,c) >= maxVal) {
                img.at<cv::Scalar>(r,c)= cv::Scalar(0,0,0, 255); //Rahul added alpha of 255 on Jan 21 2019, not sure if it still works
            }
        }
    }
    return img;
}

// takes in processed fgmask and frameDisplay to write on
// if too many pixels in fgmask are foreground, means background detection messed up
// write a warning
void checkBackground(Mat& fgmask, Mat& frameDisplay, double ratio) {
    double nonzeroratio = float(cv::countNonZero(fgmask))/(fgmask.rows*fgmask.cols);
    if (nonzeroratio > 0.75) { // bad background substraction
        badBackground = true;
        /*if (stackedUpText) {
            double fontSize = writeText(frameDisplay, firstline_badBackground);
            writeText(frameDisplay, secondline_badBackground,-1,-1, fontSize );
        } else {
            //double fontSize = writeText(frameDisplay, firstline_badBackground,-1,int(frameDisplay.cols*0.20) );
            //writeText(frameDisplay, secondline_badBackground,-1,int(frameDisplay.cols*0.35), fontSize );
            vector<tuple<string,cv::Scalar>> toWrite;
            toWrite =  {
                    make_tuple(firstline_badBackground, cv::Scalar(255,0,0)),
                    make_tuple(secondline_badBackground, cv::Scalar(255,0,0))
            };
            writeText_onRect(frameDisplay, toWrite);
        }*/
        if (currentStage != STAGE_REDBOX1) {
            writeBadBackgroundText(frameDisplay);
        }
    } else {
        badBackground = false;
    }
}

// writes the bad background text onto the frame
void writeBadBackgroundText(cv::Mat& frameDisplay) {
    writeText(frameDisplay, firstline_badBackground, -1, frameDisplay.rows*0.30, -1, cv::Scalar(255,0,0,255));
    writeText(frameDisplay, secondline_badBackground, -1, frameDisplay.rows*0.45, -1, cv::Scalar(255,0,0,255));
    //writeText_onRect(frameDisplay, badBackground_toWrite);
}


// Calculates the circular mean and st dev, which is a better "mean" and "stdev" for cyclical values like angles than the standard arithmetic mean and stdev
// Input and output are in arbitrary units between min and max; 0 and max/2 are completely different, but 0 and max are the same values
// If input were in degrees between 0 and 360, then min would be 0 and max would be 360.
// For hue, the min should be 0 and the max should be 180 since OpenCV represents hue this way so it is less than 255
std::tuple<double, double> circularMeanStdDev(Mat angles, int min, int max) {
    Mat angs;
    angles.convertTo(angs, CV_32FC1);
    //convert from the [min, max) scale to the radian scale, [-pi, +pi)
    angs = (angs-min)*2*M_PI/(max-min);
    //have it centered around 0 so Maclaurin series accuracy is as high as possible
    double offset = cv::mean(angs)[0];
    angs = angs - offset;

    //for formula for calculating circular mean/stdev see https://math.stackexchange.com/questions/2154174/calculating-the-standard-deviation-of-a-circular-quantity
    //for reasoning behind using Maclaurin to calculate sin/cos see http://answers.opencv.org/question/55602/sine-or-cosine-of-every-element-in-mat-c/
    //for Maclaurin series formulas, try http://people.math.sc.edu/girardi/m142/handouts/10sTaylorPolySeries.pdf
    //with three terms in the polynomial, we can get accuracy within a couple of degrees (tested empirically a couple of times) if the value is about 10 degrees away from the center, but an order of 9 or more is needed to get accurate results very far from the center
    //Mat sinValsOld = angs - angs.mul(angs).mul(angs)/6 + angs.mul(angs).mul(angs).mul(angs).mul(angs)/120;
    //Mat cosValsOld = 1 - angs.mul(angs)/2 + angs.mul(angs).mul(angs).mul(angs)/24;
    Mat angsPower = angs.clone();
    Mat sinVals = angs.clone();
    Mat cosVals = Mat::ones(angs.rows, angs.cols, angs.type());
    int maxOrder = 100;
    for (int i = 2; i <= maxOrder; i++) { //start at i=2 because we already have the 0th and 1st terms done
        angsPower = angsPower.mul(angs)/i;
        //cout << "i=" << i << " " << angsPower << endl;
        //angs = angs.mul(angs)/i;
        // at this point, angs is equal to x^i/i!
        if (i % 4 == 2) cosVals = cosVals - angsPower;
        else if (i % 4 == 3) sinVals = sinVals - angsPower;
        else if (i % 4 == 0) cosVals = cosVals + angsPower;
        else if (i % 4 == 1) sinVals = sinVals + angsPower;
        if (cv::mean(cv::abs(angsPower))[0]<0.001) {
            //cout << "end @ i=" << i << " -> ";
            break;
        }
    }

    double nVal = angs.rows*angs.cols;
    double sinAvg = cv::sum(sinVals)[0]/nVal; //[0] assumes single channel matrix "angles"
    double cosAvg = cv::sum(cosVals)[0]/nVal; //[0] assumes single channel matrix "angles"
    double norm2 = sinAvg*sinAvg + cosAvg*cosAvg; //should always be between 0 and 1, inclusive on both ends

    //norm2 should never be greater than 1 (but can be == 1), but do >= just in case of floating point error
    double stdev = norm2>=1 ? 0 : sqrt( log(1.0/norm2) ); //log represents the natural logarithm (not base 10)
    double avg = atan2(sinAvg,cosAvg);

    //undo Maclaurin series offset
    avg = avg + offset;

    //convert to original scale
    stdev =   + (stdev)*(max-min)/(2*M_PI);
    avg = min + (avg)*(max-min)/(2*M_PI);

    return std::make_tuple(avg, stdev);

}


int hist_wd = 512; int hist_ht = 400;
Mat histImage( hist_ht*2.2, hist_wd, CV_8UC3, Scalar( 0, 0, 240) );

Mat oldFgMask(120, 90, CV_8UC1, Scalar(0)); //for some reason, it was crashing for some reason when I defined it as (120, 90, CV_8UC1, 0). Sept 5th, 2019

/*
 * Applies the background subtractor
 * Input frameDisplay and frameEdit is in RGBA (not BGRA which is OpenCV's default for opencv image reading) because the images are read by native readers in Android and iOS.
 */
void preprocess(Mat& frameEdit, Mat &frameDisplay, Mat &fgmask, double ratio, int drawColor) {
    vector<cv::Point> contour;
    if (displayExtraInfoAmount > 1) {
        cv::putText(frameDisplay, std::to_string(fgLocAlg), cv::Point(30, 130), 2, 2,
                    cv::Scalar(0, 0, 255), 2);
    }
    if (fgLocAlg == 0) { //original
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 1) { //original but get contour after operations
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

    } else if (fgLocAlg == 4 || fgLocAlg == 6 || fgLocAlg == 8) { //original in hsv, but background subtraction initial phase time is not constant
        //Below are different parameters we can tweak to make this better or worse
        //const double thisLearningRate = 0.003; //learning rate should be smaller here because 1) nonzero amount will be able to detect smaller hand vibrations 2) it can be lower because are guaranteed a similar background across the last few seconds
        const double thisLearningRate = 0.2;
        const double percentAreaNeededToResetCalibration = 0.01;
        const double percentAreaOverlapToFinishRedBox = (fgLocAlg == 4 ? 0.6 : 1.0); //defined as the difference in area divided by the redbox hand size. Thus should be a lot bigger than percentAreaNeededToResetCalibration, defined relative to the whole image
        const int secondsOfRedBoxNeedToHold = 1;
        const int secondsOfBGNeedToRemoveBody = 2;


        Mat fgmaskColor;
        if (currentStage == STAGE_GAMEPLAY && (fgLocAlg == 6 || fgLocAlg == 8)) {
            cv::inRange(frameEdit, colorLowerBound, colorUpperBound, fgmaskColor);
            //bitwise_not(fgmaskColor,fgmaskColor);

            //morphologyEx(fgmaskColor, fgmaskColor, MORPH_OPEN,  calibrationKern);
            //morphologyEx(fgmaskColor, fgmaskColor, MORPH_CLOSE, calibrationKern);

            int nonzero4 = cv::countNonZero(fgmaskColor);

            getLargestContour(frameDisplay, fgmaskColor, ratio, drawColor);

            //cout << nonzero4 << endl;

        }
        if (currentStage != STAGE_GAMEPLAY || (fgLocAlg == 4 || fgLocAlg == 8) ) {
            cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
            fgbg->apply(frameEdit, fgmask, learning_rate < 1e-8 ? 0 : thisLearningRate); // apply background subtractor

            int nonzero1 = cv::countNonZero(fgmask);


            Mat frameEditHSV;
            cv::cvtColor(frameEdit, frameEditHSV, COLOR_RGB2HSV);
            Mat fgmask_h;
            vector<Mat> hsv;//Mat bgrHsv[3];
            split(frameEditHSV,hsv);
            fgbg_h->apply(hsv[0], fgmask_h, learning_rate); // apply background subtractor
            if (double(nonzero1)/(fgmask.cols*fgmask.rows) > 0.6) {
                fgmask = fgmask_h;
            }


            // Institute a kernel in order to remove the noise in the mask,
            // specifically removing the dots of mask in the background
            morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
            morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);


            int nonzero2 = cv::countNonZero(fgmask);

            getLargestContour(frameDisplay, fgmask, ratio, drawColor);

            int nonzero3 = cv::countNonZero(fgmask);


            if (displayExtraInfoAmount > 1) {
                std::stringstream ss1, ss2, ss3;

                //ss1 << std::setw(5) << std::setfill('0') << nonzero1;
                //cout << ss1.str() << " ";

                //ss2 << std::setw(5) << std::setfill('0') << nonzero2;
                //cout << ss2.str() << " ";

                //ss3 << std::setw(5) << std::setfill('0') << nonzero3;
                //cout << ss3.str() << endl;
            }

            if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
                if (nonzero1*1.0/(fgmask.rows*fgmask.cols) > percentAreaNeededToResetCalibration ) {
                    //commenting this out and green tint in the else statement because Anita didn't like this, but liked the redbox part
                    //calibStartTime = -secondsOfBGNeedToRemoveBody; // set it to secondsOfBGNeedToRemoveBody seconds ago, aka 5-secondsOfBGNeedToRemoveBody seconds of the timer left();
                } else {
                    //useGreenTint = 2;
                }

            } else if (currentStage == STAGE_REDBOX1 && sizeRefImgAlpha.data != NULL) {
                int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

                //Check the center of fgmask to see if it is in the redbox area
                //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
                Moments m = moments(fgmask,true);
                Point p(m.m10/m.m00, m.m01/m.m00);
                /*
                bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
                bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
                bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
                bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

                //On Sept 8th, 2019, I (Rahul) added the abs(.) because I think it was a bug that had somehow gone unnoticed for a while... ?. I added it each time this type of test came up (total of 4 matches, including the 4th one I added that day for alg 3000).
                if ( abs(nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > percentAreaOverlapToFinishRedBox || !isCenterCorrect) { //checks if hand is significantly bigger than reference hand
                    calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
                } else if ( (fgLocAlg == 6 || fgLocAlg == 8) && (nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef < 0) { //checks if the hand is smaller than the reference
                    calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
                } else {
                    useGreenTint = 2;
                }

                Mat redboxCheckArea;
                //no point in eroding because the check part isn't actually used in the game play stage
                //cv::erode( sizeRefImgAlpha, redboxCheckArea, calibrationKernBigger );
                redboxCheckArea = sizeRefImgAlpha.clone();
                //redboxCheckArea.setTo(200);
                if (displayExtraInfoAmount > 0) {
                    Mat redboxCheckAreaDraw;
                    cv::cvtColor(redboxCheckArea, redboxCheckAreaDraw, COLOR_GRAY2RGB);
                    drawImageOptimized(frameDisplay, redboxCheckAreaDraw, frameDisplay.cols/2-redboxCheckAreaDraw.cols/2, frameDisplay.rows-redboxCheckAreaDraw.rows);
                }
                cv::resize(redboxCheckArea, redboxCheckArea, cv::Size(), ratio, ratio);
                int top = frameEdit.rows-redboxCheckArea.rows;
                int bottom = 0;
                int left  = (frameEdit.cols-redboxCheckArea.cols)/2;
                int right = frameEdit.cols-redboxCheckArea.cols-left;
                cv::copyMakeBorder(redboxCheckArea, redboxCheckArea, top, bottom, left, right, BORDER_CONSTANT, Scalar(0) );
                if (displayExtraInfoAmount > 0 && false) {
                    cv::rectangle(frameDisplay, cv::Point(left / ratio, top / ratio),
                                  cv::Point((redboxCheckArea.cols - right) / ratio,
                                            (redboxCheckArea.rows - bottom) / ratio), CLR_BLACK_4UC, 10);
                }
                if (fgLocAlg == 6 || fgLocAlg == 8) {
                    Scalar mean, stdev;
                    cv::meanStdDev(frameEdit, mean, stdev, redboxCheckArea);

                    colorLowerBound = (mean - stdev * 1)-Scalar(20,20,20,0);
                    colorUpperBound = (mean + stdev * 1)+Scalar(20,20,20,0);
                    colorLowerBound[3] = 0; //alpha
                    colorUpperBound[3] = 255; //alpha

                    cout << "average: " << (mean) << "   ";
                    cout << "stdev: " << stdev << endl;

                    /*if (colorLowerBound[0] < 0) colorLowerBound[0] = 0;
                    if (colorLowerBound[1] < 0) colorLowerBound[1] = 0;
                    if (colorLowerBound[2] < 0) colorLowerBound[2] = 0;
                    if (colorUpperBound[0] > 255) colorUpperBound[0] = 255;
                    if (colorUpperBound[1] > 255) colorUpperBound[1] = 255;
                    if (colorUpperBound[2] > 255) colorUpperBound[2] = 255;*/
                }
                //checkBackground(fgmask,frameDisplay,ratio);
            }
        }

        if (currentStage == STAGE_GAMEPLAY && fgLocAlg == 6) { //ignore the MOG2 result and rewrite fgmask
            fgmask = fgmaskColor;
        } else if (currentStage == STAGE_GAMEPLAY && fgLocAlg == 8) {
            cv::bitwise_and(fgmask, fgmaskColor, fgmask);
        }

        if (displayExtraInfoAmount > 0) cout << colorLowerBound <<  " " << colorUpperBound << " " << (cv::countNonZero(fgmask)*1.0/(sizeRefImgAlpha.cols*sizeRefImgAlpha.rows)) << endl;



    } else if (fgLocAlg == 10) { //original, but on the larger non-shrunk down image
        Mat fgmask_big;

        cv::GaussianBlur( frameDisplay, frameDisplay, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameDisplay, fgmask_big, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask_big, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask_big, fgmask_big, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask_big, fgmask_big, MORPH_CLOSE, calibrationKern);

        cv::resize(fgmask_big, fgmask, cv::Size(), ratio, ratio, cv::INTER_NEAREST);

    } else if (fgLocAlg == 20) { //original with converting to hsv
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    }else if (fgLocAlg == 21) { //original with converting to hsv but switched order

        Mat fgmask_h;
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        //cv::imshow("f",fgmask);
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);

        //dilate( fgmask, fgmask, calibrationKern );
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg_h->apply(hsv[0], fgmask_h, learning_rate); // apply background subtractor
        //cv::imshow("h",fgmask_h);
        double nonzeroratio = float(cv::countNonZero(fgmask))/(fgmask.rows*fgmask.cols);
        if (nonzeroratio > 0.5) { // bad background substraction
            morphologyEx(fgmask_h, fgmask_h, MORPH_OPEN,  calibrationKern);
            morphologyEx(fgmask_h, fgmask_h, MORPH_CLOSE, calibrationKern);
            getLargestContour(frameDisplay, fgmask_h, ratio, drawColor);
            fgmask = fgmask_h.clone();
        }
    } else if (fgLocAlg== 22) { // look at hmask, get convexity defects
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(hsv[0], fgmask, learning_rate); // apply background subtractor
        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

        // find largest contour
        vector<vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        cv::findContours(fgmask,contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
        double max_area = 0;
        int max_ct_ind = -1;
        for (int i = 0; i < contours.size(); i++) {
            double ct_area = cv::contourArea(contours[i]);
            if (ct_area > max_area) {
                max_area = ct_area;
                max_ct_ind = i;
            }
        }
        if (max_area/(fgmask.rows*fgmask.cols) < 0.05) { // prevent small objects from being detected
            vector<cv::Point> empty;
            //return empty; //cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        }
        if (max_ct_ind != -1) {
            fgmask.setTo(0); //fgmask= cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));


            cv::drawContours(fgmask, contours, max_ct_ind, 255, -1);
            for(int i1=0; i1<contours.size(); i1++) {
                for(int i2=0; i2<contours[i1].size(); i2++) {
                    contours[i1][i2] = contours[i1][i2]/ratio;
                }
            }

            vector<vector<int>> hull(1);
            convexHull(Mat(contours[max_ct_ind]), hull[0], false, false);
            //cv::drawContours(fgmask, hull, 0, 255, -1);



            //draw an outline of the hand
            if (learning_rate < 0.05) {

                // convexity defect
                vector<vector<Vec4i>> defects(1);
                if(contours[max_ct_ind].size() > 3 && hull[0].size() > 3 ) // You need more than 3 indices
                {

                    cout << contours[max_ct_ind].size() << endl;
                    convexityDefects(contours[max_ct_ind], hull[0], defects[0]);
                    // draw convexity defect
                    for(const Vec4i& v : defects[0])
                    {
                        float depth = v[3] / 256;
                        if (depth > 10) //  filter defects by depth, e.g more than 10
                        {
                            int startidx = v[0]; Point ptStart(contours[max_ct_ind][startidx]);
                            int endidx = v[1]; Point ptEnd(contours[max_ct_ind][endidx]);
                            int faridx = v[2]; Point ptFar(contours[max_ct_ind][faridx]);

                            //line(frameDisplay, ptStart, ptEnd, Scalar(0, 255, 0), 10);
                            //line(frameDisplay, ptStart, ptFar, Scalar(0, 255, 0), 10);
                            //line(frameDisplay, ptEnd, ptFar, Scalar(0, 255, 0), 10);
                            circle(frameDisplay, ptFar, 10, Scalar(0, 0, 255), -1);
                            circle(frameDisplay, ptStart, 10, Scalar(0, 255, 0), -1);
                            circle(frameDisplay, ptEnd, 10, Scalar(255, 0, 0), -1);
                        }
                    }
                }

                if (drawColor == 1) { // motion is correct, draw green
                    cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(0, 255, 0, 255),
                                     4); //color is green
                } else if (drawColor == -1){ // motion is incorrect, draw red
                    cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(255, 0, 0, 255),
                                     4); //color is red
                } else if (drawColor == 0) {
                    cv::drawContours(frameDisplay, contours, max_ct_ind, Scalar(255, 255, 255, 255),
                                     4); //color is white
                }
                //cv::polylines( frameDisplay, Mat( contours[ct_ind] ) * ratio, true, Scalar(127,255,255,255), 10); // draws contour resized 2x
                //return contours[max_ct_ind];
            }

            //return cimg;
            //vector<cv::Point> empty;
            //return empty;
        }
    } else if (fgLocAlg == 23) { //original with converting to hsv, thresholding by h and looking by contours
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        Mat gray;
        cvtColor( frameEdit, gray, COLOR_RGB2GRAY );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(hsv[0], fgmask, learning_rate); // apply background subtractor
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        //cv::imshow("h",hsv[0]);
        //double minVal, maxVal;
        //Point maxDistPt; // inscribed circle center
        //minMaxLoc(fgmask, &minVal, &maxVal, NULL, &maxDistPt);
        //Scalar hsv_upper_l(minVal, 0, 0);
        //Scalar hsv_upper_h(maxVal, 255, 255);
        cv::Scalar  mean = cv::mean(frameEdit,fgmask);
        //Scalar hsv_upper_l(mean[0]-mean[0], 0, 0);
        //Scalar hsv_upper_h(mean[0]+mean[0], 255, 255);
        Mat inRan;
        inRange(frameEdit, mean*0.5, mean*1.5, inRan);
        //cv::imshow("inRan",inRan);

        vector<vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        cv::findContours(inRan,contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
        map<int, bool> cnts_to_draw;
        for (int i = 0; i < contours.size(); i++) {
            cnts_to_draw.insert(pair<int, bool>(i, false));
        }
        Mat drawing = Mat::zeros( fgmask.size(), CV_8UC3 );
        cv::drawContours(drawing, contours, -1, cv::Scalar(255,0,0), 5);
        for (int r = 0; r < fgmask.rows; r++) {
            for (int c = 0; c < fgmask.cols; c++) {

                for (int i = 0; i < contours.size(); i++) {
                    if (!cnts_to_draw.at(i) && fgmask.at<uchar>(r, c) > 0) {
                        if ((float) pointPolygonTest(contours[i], Point2f((float) c, (float) r),
                                                     false) >= 0) {
                            cv::drawContours(drawing, contours, i, cv::Scalar(0,255,0), -1);
                            //if () {
                            cnts_to_draw.at(i) = true;
                            cv::drawContours(fgmask, contours, i, 255, -1);
                            cv::fillPoly(drawing,contours,cv::Scalar(0,0,255));
                            for(int i2=0; i2<contours[i].size(); i2++) {
                                contours[i][i2] = contours[i][i2]/ratio;
                             }
                            cv::drawContours(frameDisplay, contours, i, cv::Scalar(0,255,0), 70);
                            //}
                        }
                    }
                }

            }
        }

        cv::resize(drawing,drawing,Size(),3,3);
        //cv::imshow("d",drawing);
        //cv::imshow("a",fgmask);

        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background

        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

    } else if (fgLocAlg == 24) { //original with converting to hsv, taking canny detection and looking by contours
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        Mat gray;
        cvtColor( frameEdit, gray, COLOR_RGB2GRAY );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(hsv[0], fgmask, learning_rate); // apply background subtractor
        Mat fgmask_h = fgmask.clone();
        fgmask.setTo(0);
        //morphologyEx(fgmask_h, fgmask_h, MORPH_OPEN,  calibrationKern);
        //morphologyEx(fgmask_h fgmask_h, MORPH_CLOSE, calibrationKern);

        Mat detected_edges;
        Canny( gray, detected_edges, 100, 200, 5 );
        //cv::imshow("edges", detected_edges);
        //cv::imshow("b",fgmask_h);
        vector<vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        cv::findContours(detected_edges,contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
        map<int, bool> cnts_to_draw;
        for (int i = 0; i < contours.size(); i++) {
            cnts_to_draw.insert(pair<int, bool>(i, false));
        }
        Mat drawing = Mat::zeros( fgmask.size(), CV_8UC3 );
        cv::drawContours(drawing, contours, -1, cv::Scalar(255,0,0), 5);
        for (int r = 0; r < fgmask.rows; r++) {
            for (int c = 0; c < fgmask.cols; c++) {

                for (int i = 0; i < contours.size(); i++) {
                    if (!cnts_to_draw.at(i) && fgmask_h.at<uchar>(r, c) > 0) {
                        if ((float) pointPolygonTest(contours[i], Point2f((float) c, (float) r),
                                                     false) >= 0) {
                            cv::drawContours(drawing, contours, i, cv::Scalar(0,255,0), -1);
                            //if () {
                            cnts_to_draw.at(i) = true;
                            cv::drawContours(fgmask, contours, i, 255, -1);
                            cv::fillPoly(drawing,contours,cv::Scalar(0,0,255));
                            for(int i2=0; i2<contours[i].size(); i2++) {
                                contours[i][i2] = contours[i][i2]/ratio;
                            }
                            cv::drawContours(frameDisplay, contours, i, cv::Scalar(0,255,0), 70);
                            //}
                        }
                    }
                }

            }
        }

        cv::resize(drawing,drawing,Size(),3,3);
        //cv::imshow("d",drawing);
        //cv::imshow("a",fgmask);

        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background

        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);

    } else if (fgLocAlg == 25) { //original with converting to hsv and only taking the h channel
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        /*double minVal, maxVal;
        Point maxDistPt; // inscribed circle center
        minMaxLoc(hsv[0], &minVal, &maxVal, NULL, &maxDistPt);
        cout << "Max: " << maxVal << ", Min: " << minVal << endl;
        cv::imshow("o",hsv[0]);
        cv::Mat shiftedH = hsv[0].clone();
        int shift = 25; // in openCV hue values go from 0 to 180 (so have to be doubled to get to 0 .. 360) because of byte range from 0 to 255
        for(int j=0; j<shiftedH.rows; ++j)
            for(int i=0; i<shiftedH.cols; ++i)
            {
                shiftedH.at<unsigned char>(j,i) = (shiftedH.at<unsigned char>(j,i) + shift)%180;
            }
            cv::imshow("n",shiftedH);
            hsv[0] = shiftedH;*/
        fgbg->apply(hsv[0], fgmask, learning_rate); // apply background subtractor
        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);

        //dilate( fgmask, fgmask, calibrationKern );
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        getLargestContour(frameDisplay, fgmask, ratio, drawColor, prev25);

    } else if (fgLocAlg == 26) { //original with converting to hsv and only taking the s channel
        // blows up on this one
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(hsv[1], fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 27) { //original with converting to hsv and only taking the v channel
        // blows up for this one a little
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(hsv[2], fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 28) { //original with converting to hsv and having 3 separate per channel

        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        //filterOutBrightest(frameEdit);
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        Mat fgmask_h = fgmask.clone();
        Mat fgmask_s = fgmask.clone();
        Mat fgmask_v = fgmask.clone();
        fgbg_h->apply(hsv[0], fgmask_h, learning_rate); // apply background subtractor
        fgbg_s->apply(hsv[1], fgmask_s, learning_rate); // apply background subtractor
        fgbg_v->apply(hsv[2], fgmask_v, learning_rate); // apply background subtractor
        //cv::imshow("h", fgmask_h);
        //cv::imshow("s", fgmask_s);
        //cv::imshow("v", fgmask_v);
        getLargestContour(frameDisplay, fgmask_h, ratio, -10);
        getLargestContour(frameDisplay, fgmask_s, ratio, -10);
        cv::bitwise_and(fgmask_h,fgmask_s,fgmask);
        //cv::bitwise_and(fgmask,fgmask_v,fgmask);
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 29) { //original with converting to hsv
        // h channel is considered good
        // only add pixels from s and v if they are close to h
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        //filterOutBrightest(frameEdit);
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        //Mat fgmask_h = fgmask.clone();
        Mat fgmask_s = fgmask.clone();
        Mat fgmask_v = fgmask.clone();
        fgbg_h->apply(hsv[0], fgmask, learning_rate); // apply background subtractor
        fgbg_s->apply(hsv[1], fgmask_s, learning_rate); // apply background subtractor
        fgbg_v->apply(hsv[2], fgmask_v, learning_rate); // apply background subtractor
        //cv::imshow("h", fgmask);
        //cv::imshow("s", fgmask_s);
        //cv::imshow("v", fgmask_v);
        // this gets the largest contour
        // only reason we don't call get largest contour is because we need the contour for pointPolygonTest
        vector<vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        cv::findContours(fgmask,contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
        double max_area = 0;
        int max_ct_ind = -1;
        for (int i = 0; i < contours.size(); i++) {
            double ct_area = cv::contourArea(contours[i]);
            if (ct_area > max_area) {
                max_area = ct_area;
                max_ct_ind = i;
            }
        }

        //fgmask = fgmask_h.clone();
        if (max_ct_ind != -1 && max_area/(fgmask.rows*fgmask.cols) > 0.05 ) {
            fgmask.setTo(0); //fgmask= cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));

            cv::drawContours(fgmask, contours, max_ct_ind, 255, -1);
            //fgmask =  fgmask_h.clone();
            //Mat temp = fgmask.clone();
            //temp.setTo(0);
            //cv::drawContours(temp, contours, max_ct_ind, 255, -1);
            // now get the distance between each pixel in S and V and add as weighted function
            //Mat raw_dist( fgmask_h.size(), CV_32F );
            for (int r = 0; r < fgmask.rows; r++) {
                for (int c = 0; c < fgmask.cols; c++) {
                    double dist = (float)pointPolygonTest( contours[max_ct_ind], Point2f((float)c, (float)r), true );
                    double count = 0.0;
                    if (float(cv::countNonZero(fgmask_s))/(fgmask_s.rows*fgmask_s.cols) < 0.7
                        && fgmask_s.at<uchar>(r, c) > 0) {
                        count++;
                    }
                    if (float(cv::countNonZero(fgmask_v))/(fgmask_v.rows*fgmask_v.cols) < 0.7
                         && fgmask_v.at<uchar>(r, c) > 0) {
                        count++;
                    }
                    if (abs(dist) < count*0.1*fgmask.cols  && dist < 0 ) {
                        fgmask.at<uchar>(r,c) = 255;
                    }
                    /*} else if (fgmask_s.at<float>(r, c) > 0 && dist < 0 && dist > 0.1*fgmask.cols) {

                    }*/
                    /*double should_count = 0.0;
                    if (fgmask_s.at<float>(r, c) > 0) {
                        raw_dist.at<float>(r,c) = (float)pointPolygonTest( contours[max_ct_ind], Point2f((float)c, (float)r), true );
                        should_count += 122.0 / (float) pointPolygonTest(contours[max_ct_ind],
                                                                     Point2f((float) c, (float) r),
                                                                     true);
                    }
                    if (fgmask_v.at<float>(r, c) > 0) {
                        should_count += 122.0 / (float) pointPolygonTest(contours[max_ct_ind],
                                                                     Point2f((float) c, (float) r),
                                                                     true);
                    }*/
                    /*cout << should_count << endl;
                    cout << r << "," << c << endl;
                    if (should_count > 0) {
                        temp.at<float>(r, c) = should_count;
                    }*/
                }
            }

            /*double minVal, maxVal;
            Point maxDistPt; // inscribed circle center
            minMaxLoc(raw_dist, &minVal, &maxVal, NULL, &maxDistPt);
            minVal = abs(minVal);
            maxVal = abs(maxVal);

            Mat drawing = Mat::zeros( fgmask_h.size(), CV_8UC3 );

            cout << drawing.size() << endl;
            for( int i = 0; i < fgmask_h.rows; i++ )
            {
                for( int j = 0; j < fgmask_h.cols; j++ )
                {
                    if( raw_dist.at<float>(i,j) < 0 )
                    {
                        drawing.at<Vec3b>(i,j)[0] = (uchar)(255 - abs(raw_dist.at<float>(i,j)) * 255 / minVal);
                    }
                }
            }
            //cout << temp.size() << endl;
            cout << drawing.size() << endl;
            //cv::imshow("temp", temp);
            cv::imshow("drawing",drawing);*/
        }
        //getLargestContour(frameDisplay, fgmask_s, ratio, -10);
        //cv::bitwise_and(fgmask_h,fgmask_s,fgmask);
        //cv::bitwise_and(fgmask,fgmask_v,fgmask);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
    } else if (fgLocAlg == 30) { //use otsu thresholding algorithm
        Mat grayed;
        cv::cvtColor(frameEdit, grayed, COLOR_RGBA2GRAY);
        double thres_val = cv::threshold(grayed, fgmask, 0, 255, THRESH_BINARY | THRESH_OTSU); //use both binary and otsu
        cout << thres_val << endl;
        bitwise_not(fgmask,fgmask); //flip because otsu by default detects the lighter stuff

    } else if (fgLocAlg == 31) { //original with converting to hsv and only taking the h channel
        // also has another one running concurrently that just tracks movement
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        Mat movement = fgmask.clone();
        fgbg->apply(hsv[0], fgmask, learning_rate); // apply background subtractor
        fgbg_mov->apply(frameEdit, movement, 0.03); // apply background subtractor
        //cv::imshow("fgmask",fgmask);
        //cv::imshow("mov",movement);
        cv::bitwise_or(fgmask,movement,fgmask);
        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

    } else if (fgLocAlg == 40) { //use otsu thresholding algorithm if hand is present
        Mat grayed;
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, grayed, COLOR_RGBA2GRAY);
        //Use below lines to visualize what it is like to make it grayscale (need to convert back to rgba after grayscale to maintain # of channels)
        //cv::cvtColor(frameDisplay, frameDisplay, CV_RGBA2GRAY);
        //cv::cvtColor(frameDisplay, frameDisplay, CV_GRAY2RGBA);
        double thres_val = cv::threshold(grayed, fgmask, 0, 255, THRESH_BINARY | THRESH_OTSU); //use both binary and otsu

        double mod_learning_rate = learning_rate/5;
        bgOnlyOTSUThresh = bgOnlyOTSUThresh*(1-mod_learning_rate) + thres_val*mod_learning_rate;
        if (thres_val < bgOnlyOTSUThresh * .85) { //assumes that hand will always be darker
            bitwise_not(fgmask,fgmask); //flip because otsu by default detects the lighter stuff
            morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
            morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
            getLargestContour(frameDisplay, fgmask, ratio);

        } else {
            //send empty mask because hand is not present
            fgmask.setTo(0);// = cv::Mat(fgmask.rows, fgmask.cols, CV_8U, double(0));
        }

    } else if (fgLocAlg == 42 || fgLocAlg == 43 || fgLocAlg == 41) { //both MOG2 and OTSU (OTSU applied *on top of* MOG2 mask). Really impressed by how well this works.
        Mat1b fgmask_MOG;
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask_MOG, learning_rate); // apply background subtractor

        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask_MOG, fgmask_MOG, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask_MOG, fgmask_MOG, MORPH_CLOSE, calibrationKern);

        //Dilate the frame a bit to consolidate nearby contours and counteract anything MOG mistakenly thought wasn't foreground. This is okay because OTSU will undo this if not actually part of foreground. It (theoretically) is also good because OTSU might not do a proper split if it only contains the hand and doesn't know what the background is like (alt. we could just not apply OTSU at all in these cases)
        cv::dilate( fgmask_MOG, fgmask_MOG, calibrationKernBigger );

        //Get a grayscale version for OTSU because OTSU can only operate on one channel at a time
        Mat1b grayed;
        cv::cvtColor(frameEdit, grayed, COLOR_RGBA2GRAY);
        //threshold_with_mask is cv::threshold but allows a mask input. CV_THRESH_BINARY | CV_THRESH_OTSU means both BINARY and OTSU flags are on.
        threshold_with_mask( grayed, (Mat1b&) fgmask, 0, 255, THRESH_BINARY | THRESH_OTSU,  fgmask_MOG);
        //flip because otsu by default detects the lighter stuff
        //bitwise_not(fgmask,fgmask);

        //Use setTo bc threshold_with_mask doesn't touch the non-fgmask points, so we now have to set all those points to be false
        fgmask.setTo(Scalar(0), ~fgmask_MOG); //(Note: this should be Scalar(255) if done before bitwise_not


        //getAllLargeContours(Mat& frameDisplay, Mat fgmask, double min_size, double ratio)
        //getLargestContour(Mat& frameDisplay, Mat& fgmask, double ratio, int drawColor)
        //Remove all of the small contours, and draw an outline around the contours
        //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // //fgmask = getLargestContour(frameDisplay, fgmask, ratio);

        if (fgLocAlg == 41) {
            getLargestContour(frameDisplay, fgmask, ratio, drawColor);

        } else if (fgLocAlg == 42 || fgLocAlg == 43) {
            getLargestContour(frameDisplay, fgmask, ratio, drawColor);;
            //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        }


    } else if (fgLocAlg == 44) {
        Mat1b fgmask_MOG;
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask_MOG, learning_rate); // apply background subtractor


        fgmask_MOG = getAllLargeContours(frameDisplay, fgmask_MOG, 0.01, ratio);
        //fgmask_MOG = getLargestContour(frameDisplay, fgmask_MOG, ratio);

        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask_MOG, fgmask_MOG, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask_MOG, fgmask_MOG, MORPH_CLOSE, calibrationKern);

        //Dilate the frame a bit to consolidate nearby contours and counteract anything MOG mistakenly thought wasn't foreground. This is okay because OTSU will undo this if not actually part of foreground. It (theoretically) is also good because OTSU might not do a proper split if it only contains the hand and doesn't know what the background is like (alt. we could just not apply OTSU at all in these cases)
        Mat fgmask_MOG_dilated;
        cv::dilate( fgmask_MOG, fgmask_MOG_dilated, calibrationKernBigger );

        //Get a grayscale version for OTSU because OTSU can only operate on one channel at a time
        Mat1b grayed;
        cv::cvtColor(frameEdit, grayed, COLOR_RGBA2GRAY);


        double thresh, separation;
        std::tie(thresh, separation) = otsu_8u_with_mask_returning_separation(grayed, fgmask_MOG_dilated);
        thresh = cv::threshold(grayed, fgmask, thresh, 255, THRESH_BINARY);
        bitwise_not(fgmask,fgmask);
        fgmask.setTo(Scalar(0), ~fgmask_MOG_dilated);

        Mat fgmask_big;
        cv::resize(fgmask, fgmask_big, frameDisplay.size());
        frameDisplay.setTo(Scalar(255,0,0,255), fgmask_big);

        fgmask = fgmask_MOG_dilated;
        //fgmask = getAllLargeContours(frameDisplay, fgmask, 0.001, ratio);

        cout << separation << endl;


        //threshold_mod_OTSU( grayed, (Mat1b&) fgmask,  fgmask_MOG);
        //flip because otsu by default detects the lighter stuff
        //bitwise_not(fgmask,fgmask);

        //Use setTo bc threshold_with_mask doesn't touch the non-fgmask points, so we now have to set all those points to be false
        //fgmask.setTo(Scalar(0), ~fgmask_MOG); //(Note: this should be Scalar(255) if done before bitwise_not

        //Remove all of the small contours, and draw an outline around the contours

    } else if (fgLocAlg == 60) {//ximgproc
        #if MY_OS==ANDROID_OS

        #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS) && 1<0

        int region_size = frameEdit.cols*frameEdit.rows/16;
        Ptr< cv::ximgproc::SuperpixelSLIC > xp = cv::ximgproc::createSuperpixelSLIC(frameEdit, cv::ximgproc::SLIC, region_size);
        //cv::ximgproc::SuperpixelSLIC x = &xp;
        xp->iterate();

        xp->getLabelContourMask(fgmask, true);
        frameDisplay.setTo(cv::Scalar(0, 0, 255), fgmask);
        //return fgmask;

        /*int region_size = frameEdit.cols*frameEdit.rows/16;
        cv::ximgproc::SuperpixelSLIC* x = cv::ximgproc::createSuperpixelSLIC(frameEdit, cv::ximgproc::MSLIC, region_size);
        x->iterate(5);

        //cv::Mat fgmask;
        x->getLabelContourMask(fgmask, true);
        frameDisplay.setTo(cv::Scalar(0, 0, 255), fgmask);
        return fgmask;*/

        #endif


    } else if (fgLocAlg == 100) { //original, but shows/uses all contours
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 200) { //original, but if image is too big, ups the learning rate
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 300) { //uses mean shift and cam shift
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    } else if (fgLocAlg == 1050 || fgLocAlg == 1060) { //color detection during redbox (1050 is color alone, 1060 is color plus MOG2)
        Mat frameHSV;
        Mat fgbgmask;
        cv::cvtColor(frameEdit, frameHSV, COLOR_RGB2HSV);
        frameHSV = frameEdit;
        cv::GaussianBlur( frameHSV, frameHSV, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameHSV, fgbgmask, learning_rate); // apply background subtractor

        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
            fgmask = fgbgmask;

        } else if (currentStage == STAGE_REDBOX1) {
            fgmask = fgbgmask;
            int width = frameEdit.cols*.2, height = frameEdit.rows*.45;
            int x1 = frameEdit.cols/2-width/2 + width*.45;
            int x2 = frameEdit.cols/2+width/2 + width*.45;
            int y1 = frameEdit.rows - height; //frameEdit.rows/2-height/2;
            int y2 = frameEdit.rows; //frameEdit.rows/2+height/2;
            Mat rectMask = cv::Mat::zeros(frameEdit.rows, frameEdit.cols, CV_8UC1);
            cv::rectangle(frameDisplay, Point(x1/ratio,y1/ratio), Point(x2/ratio,y2/ratio), CLR_BLACK_4UC, 20);
            cv::rectangle(rectMask, Point(x1,y1), Point(x2,y2), 255, -1);

            //Scalar mean = Scalar(0,0,0,0), stdev = Scalar(0,0,0,0);
            //cv::meanStdDev(frameDisplay, mean, stdev, rectMask);
            Scalar mean, stdev;
            cv::meanStdDev(frameHSV, mean, stdev, rectMask);
            cout << "average: " << (mean) << "   ";
            cout << "m-stdev: " << (mean-stdev) << endl;

            colorLowerBound = (mean-stdev*2);
            colorUpperBound = (mean+stdev*2);

            fgmask = fgbgmask;

        } else if (currentStage == STAGE_GAMEPLAY) {
            //cv::Scalar myLowerBound = cv::Scalar(100,100,100);
            //cv::Scalar myUpperBound = cv::Scalar(100,100,100);
            cv::inRange(frameHSV, colorLowerBound, colorUpperBound, fgmask);

            if (fgLocAlg == 1060) {
                //if algorithm 1060, then it has to be in both the MOG2 mask and the color mask
                //if algorithm 1050, then the color mask is the only requirement
                cv::bitwise_and(fgmask, fgbgmask, fgmask);
            }
            //fgmask = fgbgmask;

        } else if (currentStage == STAGE_NOT_SET) { //happens on the first frame
            fgmask = fgbgmask;

        }


        //getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio);

        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);


    }  else if (fgLocAlg == 1080) { //color detection during redbox (1050 is color alone, 1060 is color plus MOG2)

        //Below are different parameters we can tweak to make this better or worse
        const double thisLearningRate = 0.003; //learning rate should be smaller here because 1) nonzero amount will be able to detect smaller hand vibrations 2) it can be lower because are guaranteed a similar background across the last few seconds
        const double percentAreaNeededToResetCalibration = 0.01;
        const double percentAreaOverlapToFinishRedBox = 0.2; //defined as the difference in area divided by the redbox hand size. Thus should be a lot bigger than percentAreaNeededToResetCalibration, defined relative to the whole image
        const int secondsOfRedBoxNeedToHold = 1;
        const int secondsOfBGNeedToRemoveBody = 3;

        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(frameEdit, fgmask, learning_rate < 1e-8 ? 0 : thisLearningRate); // apply background subtractor

        int nonzero1 = cv::countNonZero(fgmask);

        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

        int nonzero2 = cv::countNonZero(fgmask);

        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

        int nonzero3 = cv::countNonZero(fgmask);


        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
            if (nonzero1*1.0/(fgmask.rows*fgmask.cols) > percentAreaNeededToResetCalibration ) {
                calibStartTime = -secondsOfBGNeedToRemoveBody; // set it to secondsOfBGNeedToRemoveBody seconds ago, aka 5-secondsOfBGNeedToRemoveBody seconds of the timer left();
            }
            cout << nonzero1*1.0/(fgmask.rows*fgmask.cols) << endl;

        } else if (currentStage == STAGE_REDBOX1 && sizeRefImgAlpha.data != NULL) {
            int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

            //Check the center of fgmask to see if it is in the redbox area
            //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
            Moments m = moments(fgmask,true);
            Point p(m.m10/m.m00, m.m01/m.m00);
            /*
            bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
            bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
            bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
            bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

            //On Sept 8th, 2019, I (Rahul) added the abs(.) because I think it was a bug that had somehow gone unnoticed for a while... ?. I added it each time this type of test came up (total of 4 matches, including the 4th one I added that day for alg 3000).
            if ( abs(nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > percentAreaOverlapToFinishRedBox || !isCenterCorrect) {
                calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
                const int bins=25;
                MatND hist;
                int histSize = MAX( bins, 2 );
                float hue_range[] = { 0, 180 };
                const float* ranges = { hue_range };

                /// Get the Histogram and normalize it
                calcHist( &hsv[0], 1, 0, Mat(), hist, 1, &histSize, &ranges, true, false );
                normalize( hist, hist, 0, 255, NORM_MINMAX, -1, Mat() );

                /// Get Backprojection
                MatND backproj;
                calcBackProject( &hsv[0], 1, 0, hist, backproj, &ranges, 1, true );

                /// Draw the backproj
                //imshow( "BackProj", backproj );

                /// Draw the histogram
                int w = 400; int h = 400;
                int bin_w = cvRound( (double) w / histSize );
                Mat histImg = Mat::zeros( w, h, CV_8UC3 );

                for( int i = 0; i < bins; i ++ )
                { rectangle( histImg, Point( i*bin_w, h ), Point( (i+1)*bin_w, h - cvRound( hist.at<float>(i)*h/255.0 ) ), Scalar( 0, 0, 255 ), -1 ); }

                //imshow( "Histogram", histImg );
            } else {
                useGreenTint = 2;
            }

        } else if (currentStage == STAGE_GAMEPLAY) {

        }
    } else if (fgLocAlg >= 1100 && fgLocAlg < 1200) { //original with face detector
        //cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

    #if MY_OS==ANDROID_OS

    #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
        String classifierFileName;
        if (fgLocAlg == 1100) classifierFileName = "haarcascades/haarcascade_frontalface_default.xml";
        else if (fgLocAlg == 1110) classifierFileName = "haarcascades/agest.xml";
        else if (fgLocAlg == 1120) classifierFileName = "haarcascades/fist.xml";
        else if (fgLocAlg == 1130) classifierFileName = "haarcascades/palm.xml";
        if (!faceCascadeClassifierLoaded) {
            if (faceCascadeClassifier.load(classifierFileName)) faceCascadeClassifierLoaded = true;
            else {
                throw std::invalid_argument("can't load file " + classifierFileName);
                cout << "Can't find xml haar cascade file" << endl;
            }
        }

        if (faceCascadeClassifierLoaded) {
            cv::Mat frameGray;
            cv::cvtColor(frameEdit, frameGray, COLOR_BGR2GRAY);
            cv::equalizeHist(frameGray, frameGray);
            vector<Rect> faces;
            faceCascadeClassifier.detectMultiScale(frameGray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(10, 10));

            cout << "object count: " << faces.size();
            for (size_t i = 0; i < faces.size(); i++) {
                cout << ": at " << faces[i].x << " " << faces[i].y << " " << faces[i].width << " " << faces[i].height;
                if (fgLocAlg == 1100) { //block out result from the image
                    cv::rectangle( fgmask, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0), -1);
                } else { //make sure the image includes this
                    cv::rectangle( fgmask, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(255), -1);
                }
                cv::rectangle( frameDisplay, Point(faces[i].x/ratio, faces[i].y/ratio), Point(faces[i].x/ratio + faces[i].width/ratio, faces[i].y/ratio + faces[i].height/ratio), Scalar(0, 255, 255, 255), 20 );
            }
            cout << endl;
        }
    #endif




    }  else if (fgLocAlg == 1300) { //original
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );

        cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg->apply(hsv[0], fgmask, 0.02); // apply background subtractor
        //cv::imshow("m",fgmask);
        double nonzeroratio = float(cv::countNonZero(fgmask))/(fgmask.rows*fgmask.cols);
        if (nonzeroratio > 0.01) { // large enough area
            double min, max;
            cv::minMaxLoc(fgmask, &min, &max);
            cout << min << " - " << max << endl;
            cv::minMaxLoc(fgmask, &lowerBoundH, &upperBoundH, NULL,NULL);
        }
        cout << nonzeroratio << ": " << lowerBoundH << " , "  << upperBoundH <<  endl;
        cv::inRange(hsv[0], lowerBoundH, upperBoundH, fgmask);
        getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        drawHistogram(frameEdit,true);
        Mat frame_with_mask = Mat::zeros( fgmask.size(), CV_8UC3 );
        cv::bitwise_and(frameEdit,frameEdit,frame_with_mask,fgmask);

        Mat gray;
        cvtColor(frame_with_mask,frame_with_mask,COLOR_HSV2RGB);
        cvtColor( frame_with_mask, gray, COLOR_RGB2GRAY );
        Mat detected_edges;
        Canny( gray, detected_edges, 100, 200, 5 );
        //cv::imshow("edges", detected_edges);
        //cv::imshow("fmw",frame_with_mask);

    } else if (fgLocAlg == 1400) { // blob detector
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        // Set up the detector with default parameters.
        //SimpleBlobDetector blobDetector;
        SimpleBlobDetector::Params params;
        Ptr<SimpleBlobDetector> blobDetector = SimpleBlobDetector::create(params);
        // Detect blobs.
        std::vector<KeyPoint> keypoints;
        blobDetector->detect( frameEdit, keypoints);


        // Draw detected blobs as red circles.
        // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
        Mat drawing = Mat::zeros( frameEdit.size(), CV_8UC3 );
        for (int i = 0; i < keypoints.size(); i++) {
            circle(drawing, keypoints[i].pt, int(keypoints[i].size), Scalar(0, 0, 255), -1);
        }
        //drawKeypoints( blobDetector, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

        // Show blobs
        //imshow("keypoints", drawing );


    } else if (fgLocAlg == 1500) { // mean shift

        //Below are different parameters we can tweak to make this better or worse
        const double thisLearningRate = 0.003; //learning rate should be smaller here because 1) nonzero amount will be able to detect smaller hand vibrations 2) it can be lower because are guaranteed a similar background across the last few seconds
        const double percentAreaNeededToResetCalibration = 0.01;
        const double percentAreaOverlapToFinishRedBox = 0.2; //defined as the difference in area divided by the redbox hand size. Thus should be a lot bigger than percentAreaNeededToResetCalibration, defined relative to the whole image
        const int secondsOfRedBoxNeedToHold = 1;
        const int secondsOfBGNeedToRemoveBody = 2;

        //float hranges[] = { 0,180 };
        //const float* phranges[] = { hranges,hranges,hranges };
        //int ch[] = { 0, 1,2 };

        //int hsize[] = { 16,16,16 };

        const int bins=25;
        int histSize = MAX( bins, 2 );
        float hue_range[] = { 0, 180 };
        const float* ranges = { hue_range };

        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );

        cvtColor(frameEdit, frameEdit, COLOR_BGR2HSV);
        fgbg->apply(frameEdit, fgmask, learning_rate < 1e-8 ? 0 : thisLearningRate); // apply background subtractor

        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);

        int nonzero1 = cv::countNonZero(fgmask);


        /*cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        Mat fgmask_h;
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg_h->apply(hsv[0], fgmask_h, learning_rate); // apply background subtractor
        if (double(nonzero1)/(fgmask.cols*fgmask.rows) > 0.6) {
            fgmask = fgmask_h;
        }*/

        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

        int nonzero2 = cv::countNonZero(fgmask);

        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

        int nonzero3 = cv::countNonZero(fgmask);


        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
            if (nonzero1*1.0/(fgmask.rows*fgmask.cols) > percentAreaNeededToResetCalibration ) {
                //commenting this out and green tint in the else statement because Anita didn't like this, but liked the redbox part
                //calibStartTime = -secondsOfBGNeedToRemoveBody; // set it to secondsOfBGNeedToRemoveBody seconds ago, aka 5-secondsOfBGNeedToRemoveBody seconds of the timer left();
            } else {
                //useGreenTint = 2;
            }

        } else if (currentStage == STAGE_REDBOX1 && sizeRefImgAlpha.data != NULL) {
            int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

            //Check the center of fgmask to see if it is in the redbox area
            //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
            Moments m = moments(fgmask,true);
            Point p(m.m10/m.m00, m.m01/m.m00);
            /*
            bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
            bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
            bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
            bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

            if ( (nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > 3*percentAreaOverlapToFinishRedBox || !isCenterCorrect) {
                calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
            } else {
                useGreenTint = 2;
            }

            Mat redboxCheckArea;
            //no point in eroding because the check part isn't actually used in the game play stage
            //cv::erode( sizeRefImgAlpha, redboxCheckArea, calibrationKernBigger );
            redboxCheckArea = sizeRefImgAlpha.clone();
            redboxCheckArea.setTo(200);
            if (displayExtraInfoAmount > 0) {
                Mat redboxCheckAreaDraw;
                cv::cvtColor(redboxCheckArea, redboxCheckAreaDraw, COLOR_GRAY2RGB);
                drawImageOptimized(frameDisplay, redboxCheckAreaDraw, frameDisplay.cols/2-redboxCheckAreaDraw.cols/2, frameDisplay.rows-redboxCheckAreaDraw.rows);
            }
            cv::resize(redboxCheckArea, redboxCheckArea, cv::Size(), ratio, ratio);
            int top = frameEdit.rows-redboxCheckArea.rows;
            int bottom = 0;
            int left  = (frameEdit.cols-redboxCheckArea.cols)/2;
            int right = frameEdit.cols-redboxCheckArea.cols-left;
            cv::copyMakeBorder(redboxCheckArea, redboxCheckArea, top, bottom, left, right, BORDER_CONSTANT, Scalar(0,0,0,0) );
            if (displayExtraInfoAmount > 0) {
                cv::rectangle(frameDisplay, cv::Point(left / ratio, top / ratio),
                              cv::Point((redboxCheckArea.cols - right) / ratio,
                                        (redboxCheckArea.rows - bottom) / ratio), CLR_BLACK_4UC, 10);

            }
            // mean shift
            trackWindow.x = int(left*2);
            trackWindow.y = int(top*2);
            trackWindow.width = int(redboxCheckArea.cols*0.7) - right-1;
            trackWindow.height = int(frameEdit.rows*0.7) - top - 1;

            cout << trackWindow.x << "," << trackWindow.y << "," << trackWindow.width << "," << trackWindow.height << endl;

            Rect drawTrack;
            drawTrack.width = int(trackWindow.width/ratio);
            drawTrack.height = int(trackWindow.height/ratio);
            drawTrack.x= int(trackWindow.x/ratio);
            drawTrack.y = int(trackWindow.y/ratio);
            cv::rectangle(frameDisplay, drawTrack, Scalar(255, 128, 128), 10);

            Mat roi = hsv[0](trackWindow);

            Mat maskroi;
            inRange(roi, Scalar(0., 60., 32.), Scalar(180., 255., 255.), maskroi);
            //calcHist(&roi, 1, 0, maskroi, roi_hist_meanShift, 1, hsize, phranges);
            //normalize(roi_hist_meanShift, roi_hist_meanShift, 0, 255, NORM_MINMAX);

            /// Get the Histogram and normalize it
            calcHist( &roi, 1, 0, Mat(), roi_hist_meanShift, 1, &histSize, &ranges, true, false );
            normalize( roi_hist_meanShift, roi_hist_meanShift, 0, 255, NORM_MINMAX, -1, Mat() );
        } else if (currentStage == STAGE_GAMEPLAY) {
            Mat dst;

            /// Get Backprojection
            calcBackProject( &hsv[0], 1, 0, roi_hist_meanShift, dst, &ranges, 1, true );
            //cv::imshow("back",dst);
            /*fgmask = dst;
            //calcBackProject(&frameEdit, 1, ch, roi_hist_meanShift, dst, phranges);


            meanShift(dst, trackWindow, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1));
            Rect drawTrack;
            drawTrack.width = int(trackWindow.width/ratio);
            drawTrack.height = int(trackWindow.height/ratio);
            drawTrack.x= int(trackWindow.x/ratio);
            drawTrack.y = int(trackWindow.y/ratio);
            rectangle(frameDisplay, drawTrack, Scalar(255, 128, 128), 12);*/
        }
    } else if (fgLocAlg == 1550 || fgLocAlg == 1551) { // 1550= kcf, 1551 = CSRT

#if MY_OS==LINUX_OS
        //Below are different parameters we can tweak to make this better or worse
        const double thisLearningRate = 0.003; //learning rate should be smaller here because 1) nonzero amount will be able to detect smaller hand vibrations 2) it can be lower because are guaranteed a similar background across the last few seconds
        const double percentAreaNeededToResetCalibration = 0.01;
        const double percentAreaOverlapToFinishRedBox = 0.2; //defined as the difference in area divided by the redbox hand size. Thus should be a lot bigger than percentAreaNeededToResetCalibration, defined relative to the whole image
        const int secondsOfRedBoxNeedToHold = 1;
        const int secondsOfBGNeedToRemoveBody = 2;


        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );

        //cvtColor(frameEdit, frameEdit, COLOR_BGR2HSV);
        fgbg->apply(frameEdit, fgmask, learning_rate < 1e-8 ? 0 : thisLearningRate); // apply background subtractor

        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);

        int nonzero1 = cv::countNonZero(fgmask);


        /*cv::cvtColor(frameEdit, frameEdit, COLOR_RGB2HSV);
        Mat fgmask_h;
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEdit,hsv);
        fgbg_h->apply(hsv[0], fgmask_h, learning_rate); // apply background subtractor
        if (double(nonzero1)/(fgmask.cols*fgmask.rows) > 0.6) {
            fgmask = fgmask_h;
        }*/

        getLargestContour(frameDisplay, fgmask, ratio, drawColor);

        int nonzero2 = cv::countNonZero(fgmask);

        // Institute a kernel in order to remove the noise in the mask,
        // specifically removing the dots of mask in the background
        morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
        morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);

        int nonzero3 = cv::countNonZero(fgmask);



        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
            fgbg_mov->apply(frameEdit, fgmask,0.02);
            if (nonzero1*1.0/(fgmask.rows*fgmask.cols) > percentAreaNeededToResetCalibration ) {
                //commenting this out and green tint in the else statement because Anita didn't like this, but liked the redbox part
                //calibStartTime = -secondsOfBGNeedToRemoveBody; // set it to secondsOfBGNeedToRemoveBody seconds ago, aka 5-secondsOfBGNeedToRemoveBody seconds of the timer left();
            } else {
                //useGreenTint = 2;
            }

        } else if (currentStage == STAGE_REDBOX1 && sizeRefImgAlpha.data != NULL) {
            int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

            //Check the center of fgmask to see if it is in the redbox area
            //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
            Moments m = moments(fgmask,true);
            Point p(m.m10/m.m00, m.m01/m.m00);
            /*
            bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
            bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
            bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
            bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

            if ( (nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > 3*percentAreaOverlapToFinishRedBox || !isCenterCorrect) {
                calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
            } else {
                useGreenTint = 2;
            }

            Mat redboxCheckArea;
            //no point in eroding because the check part isn't actually used in the game play stage
            //cv::erode( sizeRefImgAlpha, redboxCheckArea, calibrationKernBigger );
            redboxCheckArea = sizeRefImgAlpha.clone();
            redboxCheckArea.setTo(200);
            if (displayExtraInfoAmount > 0) {
                Mat redboxCheckAreaDraw;
                cv::cvtColor(redboxCheckArea, redboxCheckAreaDraw, COLOR_GRAY2RGB);
                drawImageOptimized(frameDisplay, redboxCheckAreaDraw, frameDisplay.cols/2-redboxCheckAreaDraw.cols/2, frameDisplay.rows-redboxCheckAreaDraw.rows);
            }
            cv::resize(redboxCheckArea, redboxCheckArea, cv::Size(), ratio, ratio);
            int top = frameEdit.rows-redboxCheckArea.rows;
            int bottom = 0;
            int left  = (frameEdit.cols-redboxCheckArea.cols)/2;
            int right = frameEdit.cols-redboxCheckArea.cols-left;
            cv::copyMakeBorder(redboxCheckArea, redboxCheckArea, top, bottom, left, right, BORDER_CONSTANT, Scalar(0,0,0,0) );
            if (displayExtraInfoAmount > 0) {
                cv::rectangle(frameDisplay, cv::Point(left / ratio, top / ratio),
                              cv::Point((redboxCheckArea.cols - right) / ratio,
                                        (redboxCheckArea.rows - bottom) / ratio), CLR_BLACK_4UC, 10);

            }
            // get bounding box
             //trackWindow.x = int(left*3);
            //trackWindow.y = int(top*4);
            //trackWindow.width = 20;//int(redboxCheckArea.cols*0.7) - right-1;
            //trackWindow.height = 20;//int(frameEdit.rows*0.7) - top - 1;
            Mat fgmask_mov;
            fgbg_mov->apply(frameEdit, fgmask_mov,0.2);
            //cv::imshow("mov",fgmask_mov);
            vector<vector<cv::Point> > contours;
            vector<cv::Vec4i> hierarchy;
            cv::findContours(fgmask_mov,contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
            double max_area = 0;
            int max_ct_ind = -1;
            for (int i = 0; i < contours.size(); i++) {
                double ct_area = cv::contourArea(contours[i]);
                if (ct_area > max_area) {
                    max_area = ct_area;
                    max_ct_ind = i;
                }
            }
            if (max_area/(fgmask_mov.rows*fgmask_mov.cols) > 0.05 && max_ct_ind != -1) {
                trackWindow = cv::boundingRect(contours[max_ct_ind]);

                cout << "Initial: " <<  trackWindow.x << "," << trackWindow.y << "," << trackWindow.width << "," << trackWindow.height << endl;

                // initialize the tracker
                if (fgLocAlg == 1550) {
                    mot_tracker = TrackerKCF::create();
                } else if (fgLocAlg == 1551) {
                    cv::cvtColor(frameEdit,frameEdit,cv::COLOR_BGRA2BGR);
                    mot_tracker = TrackerCSRT::create();
                }
                mot_tracker->init(frameEdit,trackWindow);

                Rect drawTrack;
                drawTrack.width = int(trackWindow.width/ratio);
                drawTrack.height = int(trackWindow.height/ratio);
                drawTrack.x= int(trackWindow.x/ratio);
                drawTrack.y = int(trackWindow.y/ratio);
                cv::rectangle(frameDisplay, drawTrack, Scalar(255, 128, 128), 10);
            }




          } else if (currentStage == STAGE_GAMEPLAY) {

            cout << trackWindow.x << "," << trackWindow.y << "," << trackWindow.width << "," << trackWindow.height << endl;

            cv::cvtColor(frameEdit,frameEdit,cv::COLOR_BGRA2BGR);
            // update the tracking result
            bool res = mot_tracker->update(frameEdit,trackWindow);
            cv::putText(frameDisplay, std::to_string(res), cv::Point(130,130), cv::FONT_HERSHEY_SIMPLEX,
                        4,
                        cv::Scalar(0, 0, 255), 4); //black outside
            // draw the tracked object
            Rect drawTrack;
            drawTrack.width = int(trackWindow.width/ratio);
            drawTrack.height = int(trackWindow.height/ratio);
            drawTrack.x= int(trackWindow.x/ratio);
            drawTrack.y = int(trackWindow.y/ratio);
            cv::rectangle(frameDisplay, drawTrack, Scalar(255, 128, 128), 10);
        }
#endif

    } else if (fgLocAlg == 1600 || fgLocAlg == 1604 || fgLocAlg == 1610 || fgLocAlg == 1614) {

        //Below are different parameters we can tweak to make this better or worse
        //const double thisLearningRate = 0.003; //learning rate should be smaller here because 1) nonzero amount will be able to detect smaller hand vibrations 2) it can be lower because are guaranteed a similar background across the last few seconds
        const double thisLearningRate = 0.2;
        const double percentAreaNeededToResetCalibration = 0.01;
        const double percentAreaOverlapToFinishRedBox = 1.0; //defined as the difference in area divided by the redbox hand size. Thus should be a lot bigger than percentAreaNeededToResetCalibration, defined relative to the whole image
        const int secondsOfRedBoxNeedToHold = 1;
        const int secondsOfBGNeedToRemoveBody = 2;


        Mat fgmask_standard;
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask_standard, learning_rate < 1e-8 ? 0 : thisLearningRate); // apply background subtractor

        int nonzero1 = cv::countNonZero(fgmask_standard);


        Mat frameEditHSV;
        cv::cvtColor(frameEdit, frameEditHSV, COLOR_RGB2HSV);
        Mat fgmask_h;
        vector<Mat> hsv;//Mat bgrHsv[3];
        split(frameEditHSV,hsv);
        fgbg_h->apply(hsv[0], fgmask_h, learning_rate); // apply background subtractor

        vector<Mat> channels(frameEditHSV.channels());
        // split img:
        split(frameEditHSV, channels);

#if (MY_OS==MAC_OS || MY_OS==LINUX_OS) && false
        //Make histogram (will draw it later)
        //Source: https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html

        int histSize = 256; //from 0 to 255
        float range[] = { 0, 256 } ; //the upper boundary is exclusive
        int bin_wd = cvRound( (double) hist_wd/histSize );

        const float* histRange = { range }; /// Set the ranges ( for B,G,R or H,S,V etc) )
        bool uniform = true;
        bool accumulate = false;
        Scalar meanClr = Scalar(0, 255, 200);

        Mat h_hist, s_hist, v_hist;
        cv::calcHist( &channels[0], 1, 0, Mat(), h_hist, 1, &histSize, &histRange, uniform, accumulate );
        cv::calcHist( &channels[1], 1, 0, Mat(), s_hist, 1, &histSize, &histRange, uniform, accumulate );
        cv::calcHist( &channels[2], 1, 0, Mat(), v_hist, 1, &histSize, &histRange, uniform, accumulate );

        /// Normalize the result to [ 0, hist_ht ]
        normalize(h_hist, h_hist, 0, hist_ht, NORM_MINMAX, -1, Mat() );
        normalize(s_hist, s_hist, 0, hist_ht, NORM_MINMAX, -1, Mat() );
        normalize(v_hist, v_hist, 0, hist_ht, NORM_MINMAX, -1, Mat() );

        cv::Mat redrawMaskTop = cv::Mat::zeros(histImage.size(), CV_8U); // all 0
        redrawMaskTop( Rect(0,0,hist_wd,hist_ht*1.1) ) = 1;
        histImage.setTo( Scalar( 0, 0, 240), redrawMaskTop );

        cv::putText(histImage, "Entire Frame", cv::Point(hist_wd*0.1,hist_ht*0.08), FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,0), 2);

        for( int i = 1; i < histSize; i++ ) {
            /*
             cv::line( histImage, Point( bin_wd*(i-1), hist_ht - cvRound(h_hist.at<float>(i-1)) ) ,
             Point( bin_wd*(i), hist_ht - cvRound(h_hist.at<float>(i)) ),
             Scalar( i, meanClr[1], meanClr[2]), 2, 8, 0  );
             cv::line( histImage, Point( bin_wd*(i-1), hist_ht - cvRound(s_hist.at<float>(i-1)) ) ,
             Point( bin_wd*(i), hist_ht - cvRound(s_hist.at<float>(i)) ),
             Scalar( meanClr[0], i, meanClr[2]), 2, 8, 0  );
             cv::line( histImage, Point( bin_wd*(i-1), hist_ht - cvRound(v_hist.at<float>(i-1)) ) ,
             Point( bin_wd*(i), hist_ht - cvRound(v_hist.at<float>(i)) ),
             Scalar( meanClr[0], meanClr[1], i), 2, 8, 0  );*/
            cv::line( histImage, Point( bin_wd*(i-1), hist_ht*1.1 - cvRound(h_hist.at<float>(i-1)) ) ,
                     Point( bin_wd*(i), hist_ht*1.1 - cvRound(h_hist.at<float>(i)) ),
                     Scalar( 230/2, meanClr[1], meanClr[2]), 2, 8, 0  );
            cv::line( histImage, Point( bin_wd*(i-1), hist_ht*1.1 - cvRound(s_hist.at<float>(i-1)) ) ,
                     Point( bin_wd*(i), hist_ht*1.1 - cvRound(s_hist.at<float>(i)) ),
                     Scalar( 120/2, meanClr[1], meanClr[2]), 2, 8, 0  );
            cv::line( histImage, Point( bin_wd*(i-1), hist_ht*1.1 - cvRound(v_hist.at<float>(i-1)) ) ,
                     Point( bin_wd*(i), hist_ht*1.1 - cvRound(v_hist.at<float>(i)) ),
                     Scalar( 0/2, meanClr[1], meanClr[2]), 2, 8, 0  );
        }
#endif


        //Do this for both subtraction and redbox
        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD || currentStage == STAGE_REDBOX1) {
            if (double(nonzero1)/(fgmask_standard.cols*fgmask_standard.rows) > 0.6) {
                fgmask = fgmask_h;
            } else {
                fgmask = fgmask_standard;
            }

            getLargestContour(frameDisplay, fgmask, ratio, drawColor);

            int nonzero2 = cv::countNonZero(fgmask);

            // Institute a kernel in order to remove the noise in the mask,
            // specifically removing the dots of mask in the background
            morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
            morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        }


        //Split up based on stage
        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
            if (backgroundExample.data == NULL) {
                backgroundExample = frameEditHSV;
                backgroundExample.convertTo(backgroundExample, CV_32FC4);
            } else {
                accumulateWeighted(frameEditHSV, backgroundExample, 0.01); //now no error here (i hope so)
                //cout << "mean " << cv::mean(backgroundExample) << endl;

            }

        } else if (currentStage == STAGE_REDBOX1) {
            if (sizeRefImgAlpha.data != NULL) {
                int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

                //Check the center of fgmask to see if it is in the redbox area
                //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
                Moments m = moments(fgmask,true);
                Point p(m.m10/m.m00, m.m01/m.m00);
                /*
                bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
                bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
                bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
                bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzeroSizeRef>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;


                Mat redboxCheckArea;
                //no point in eroding because the check part isn't actually used in the game play stage
                cv::erode( sizeRefImgAlpha, redboxCheckArea, calibrationKernBigger );
                //redboxCheckArea = sizeRefImgAlpha.clone();
                //redboxCheckArea.setTo(200);
                if (displayExtraInfoAmount > 0) {
                    Mat redboxCheckAreaDraw;
                    cv::cvtColor(redboxCheckArea, redboxCheckAreaDraw, COLOR_GRAY2RGB);
                    drawImageOptimized(frameDisplay, redboxCheckAreaDraw, frameDisplay.cols/2-redboxCheckAreaDraw.cols/2, frameDisplay.rows-redboxCheckAreaDraw.rows);
                }
                cv::resize(redboxCheckArea, redboxCheckArea, cv::Size(), ratio, ratio);
                int top = frameEditHSV.rows-redboxCheckArea.rows;
                int bottom = 0;
                int left  = (frameEditHSV.cols-redboxCheckArea.cols)/2;
                int right = frameEditHSV.cols-redboxCheckArea.cols-left;
                cv::copyMakeBorder(redboxCheckArea, redboxCheckArea, top, bottom, left, right, BORDER_CONSTANT, Scalar(0) );
                if (displayExtraInfoAmount > 0 && false) {
                    cv::rectangle(frameDisplay, cv::Point(left / ratio, top / ratio),
                                  cv::Point((redboxCheckArea.cols - right) / ratio,
                                            (redboxCheckArea.rows - bottom) / ratio), CLR_BLACK_4UC, 10);
                }


                Scalar mean, stdev;
                cv::meanStdDev(frameEditHSV, mean, stdev, redboxCheckArea);

#if (MY_OS==MAC_OS || MY_OS==LINUX_OS) && false
                Mat h_hist_check, s_hist_check, v_hist_check;
                cv::calcHist( &channels[0], 1, 0, redboxCheckArea, h_hist_check, 1, &histSize, &histRange, uniform, accumulate );
                cv::calcHist( &channels[1], 1, 0, redboxCheckArea, s_hist_check, 1, &histSize, &histRange, uniform, accumulate );
                cv::calcHist( &channels[2], 1, 0, redboxCheckArea, v_hist_check, 1, &histSize, &histRange, uniform, accumulate );

                //int maxCheck = (int) (hist_ht*cv::countNonZero(redboxCheckArea)/(redboxCheckArea.rows*redboxCheckArea.cols));
                int maxCheck = hist_ht;
                normalize(h_hist_check, h_hist_check, 0, maxCheck, NORM_MINMAX, -1, Mat() );
                normalize(s_hist_check, s_hist_check, 0, maxCheck, NORM_MINMAX, -1, Mat() );
                normalize(v_hist_check, v_hist_check, 0, maxCheck, NORM_MINMAX, -1, Mat() );

                cv::Mat redrawMaskBottom = cv::Mat::zeros(histImage.size(), CV_8U); // all 0
                redrawMaskBottom( Rect(0,hist_ht*1.1,hist_wd,hist_ht*1.1) ) = 1;
                histImage.setTo( Scalar( 0, 0, 240), redrawMaskBottom );

                cv::putText(histImage, "Checked Area", cv::Point(hist_wd*0.1,hist_ht*1.18), FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,0), 2);

                for( int i = 1; i < histSize; i++ ) {
                    cv::line( histImage, Point( bin_wd*(i-1), hist_ht*2.2 - cvRound(h_hist_check.at<float>(i-1)) ) ,
                             Point( bin_wd*(i), hist_ht*2.2 - cvRound(h_hist_check.at<float>(i)) ),
                             Scalar( 230/2, meanClr[1], meanClr[2]), 2, 8, 0  );
                    cv::line( histImage, Point( bin_wd*(i-1), hist_ht*2.2 - cvRound(s_hist_check.at<float>(i-1)) ) ,
                             Point( bin_wd*(i), hist_ht*2.2 - cvRound(s_hist_check.at<float>(i)) ),
                             Scalar( 120/2, meanClr[1], meanClr[2]), 2, 8, 0  );
                    cv::line( histImage, Point( bin_wd*(i-1), hist_ht*2.2 - cvRound(v_hist_check.at<float>(i-1)) ) ,
                             Point( bin_wd*(i), hist_ht*2.2 - cvRound(v_hist_check.at<float>(i)) ),
                             Scalar( 0/2, meanClr[1], meanClr[2]), 2, 8, 0  );
                }

#endif

                cout << "avg org: " << mean << " ";
                cout << "std org: " << stdev << ", ";

                double hue_avg, hue_stdev;
                std::tie(hue_avg, hue_stdev) = circularMeanStdDev(channels[0], 0, 180); //note hue is circular, but is from 0 to 180, NOT 0 to 360 (designed this way to be under 255)
                mean[0] = hue_avg;
                stdev[0] = hue_stdev;

                colorLowerBound = (mean - stdev * 1.5)-Scalar(20,20,20,0);
                colorUpperBound = (mean + stdev * 1.5)+Scalar(20,20,20,0);
                colorLowerBound[3] = 0; //alpha
                colorUpperBound[3] = 255; //alpha

                cout << "avg h: " << hue_avg << "   ";
                cout << "std h: " << hue_stdev << endl;

                /*if (colorLowerBound[0] < 0) colorLowerBound[0] = 0;
                 if (colorLowerBound[1] < 0) colorLowerBound[1] = 0;
                 if (colorLowerBound[2] < 0) colorLowerBound[2] = 0;
                 if (colorUpperBound[0] > 255) colorUpperBound[0] = 255;
                 if (colorUpperBound[1] > 255) colorUpperBound[1] = 255;
                 if (colorUpperBound[2] > 255) colorUpperBound[2] = 255;*/
                //checkBackground(fgmask,frameDisplay,ratio);

                //assert foregroundExample.size() == frameEdit.size();
                foregroundExample.setTo(Scalar(0,0,0,0));
                frameEditHSV.copyTo(foregroundExample, redboxCheckArea);
            }



        } else if (currentStage == STAGE_GAMEPLAY) {
            Mat fgmask1, fgmask2, fgmask3;
            if (fgLocAlg == 1600 || fgLocAlg == 1604) {
                Scalar modLowerBound = colorLowerBound;
                Scalar modUpperBound = colorUpperBound;
                //Need to add the +/-180 ranges to factor in the fact that Hue is circular (i.e. 181 is the same 1, and therefore red is split between the top and bottom of the H spectrum)
                cv::inRange(frameEditHSV, modLowerBound,                   modUpperBound,                   fgmask1);
                cv::inRange(frameEditHSV, modLowerBound-Scalar(180,0,0,0), modUpperBound-Scalar(180,0,0,0), fgmask2);
                cv::inRange(frameEditHSV, modLowerBound+Scalar(180,0,0,0), modUpperBound+Scalar(180,0,0,0), fgmask3);

                // | is the logical OR operator, and & is the logical AND operator
                if (fgLocAlg == 1600) {
                    fgmask = (fgmask1 | fgmask2 | fgmask3);
                } else if (fgLocAlg == 1604) {
                    fgmask = (fgmask1 | fgmask2 | fgmask3 | fgmask_h) & fgmask_standard;
                }


            } else if (fgLocAlg == 1610 || fgLocAlg == 1614) {
                Scalar modLowerBound = cv::Scalar(colorLowerBound[0],   100,   0);
                Scalar modUpperBound = cv::Scalar(colorUpperBound[0],   255, 255);
                //Need to add the +/-180 ranges to factor in the fact that Hue is circular (i.e. 181 is the same 1, and therefore red is split between the top and bottom of the H spectrum)
                cv::inRange(frameEditHSV, modLowerBound,                   modUpperBound,                   fgmask1);
                cv::inRange(frameEditHSV, modLowerBound-Scalar(180,0,0,0), modUpperBound-Scalar(180,0,0,0), fgmask2);
                cv::inRange(frameEditHSV, modLowerBound+Scalar(180,0,0,0), modUpperBound+Scalar(180,0,0,0), fgmask3);

                // | is the logical OR operator, and & is the logical AND operator
                if (fgLocAlg == 1610) {
                    fgmask = (fgmask1 | fgmask2 | fgmask3);
                } else if (fgLocAlg == 1614) {
                    fgmask = (fgmask1 | fgmask2 | fgmask3 | fgmask_h) & fgmask_standard;
                }
            }


            //Don't do the morphologyEx because it removes the fingers when they are two narrow
            // Institute a kernel in order to remove the noise in the mask,
            // specifically removing the dots of mask in the background
            //morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
            //morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
            //cv::dilate( fgmask_MOG, fgmask_MOG_dilated, calibrationKernBigger );



            getLargestContour(frameDisplay, fgmask, ratio, drawColor);




        } else {
            backgroundExample.data = NULL;
            foregroundExample.data = NULL;

        }


#if (MY_OS==MAC_OS || MY_OS==LINUX_OS) && false
        Mat histImageDispl;
        cv::cvtColor(histImage, histImageDispl, COLOR_HSV2BGR);
        imshow("calcHist Demo", histImageDispl );
#endif



    } else if (fgLocAlg == 1650 || fgLocAlg == 1654 || fgLocAlg == 1660 || fgLocAlg == 1664 || fgLocAlg == 1670 || fgLocAlg == 1674 || fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694) {

        //Below are different parameters we can tweak to make this better or worse
        //const double thisLearningRate = 0.003; //learning rate should be smaller here because 1) nonzero amount will be able to detect smaller hand vibrations 2) it can be lower because are guaranteed a similar background across the last few seconds
        const double thisLearningRate = 0.2;
        const double percentAreaNeededToResetCalibration = 0.01;
        const double percentAreaOverlapToFinishRedBox = 1; //defined as the difference in area divided by the redbox hand size. Thus should be a lot bigger than percentAreaNeededToResetCalibration, defined relative to the whole image
        const int secondsOfRedBoxNeedToHold = 1;
        const int secondsOfBGNeedToRemoveBody = 2;


        Mat fgmask_standard;
        cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
        fgbg->apply(frameEdit, fgmask_standard, learning_rate < 1e-8 ? 0 : thisLearningRate); // apply background subtractor

        int nonzero1 = cv::countNonZero(fgmask_standard);


        Mat frameEditHSV;
        cv::cvtColor(frameEdit, frameEditHSV, COLOR_RGB2HSV);

        vector<Mat> hsvChannels(frameEditHSV.channels());
        // split img:
        split(frameEditHSV, hsvChannels);

#if (MY_OS==MAC_OS || MY_OS==LINUX_OS) && false
        //Make histogram (will draw it later)
        //Source: https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html

        int histSize = 256; //from 0 to 255
        float range[] = { 0, 256 } ; //the upper boundary is exclusive
        int bin_wd = cvRound( (double) hist_wd/histSize );

        const float* histRange = { range }; /// Set the ranges ( for B,G,R or H,S,V etc) )
        bool uniform = true;
        bool accumulate = false;
        Scalar meanClr = Scalar(0, 255, 200);

        Mat h_hist, s_hist, v_hist;
        cv::calcHist( &hsvChannels[0], 1, 0, Mat(), h_hist, 1, &histSize, &histRange, uniform, accumulate );
        cv::calcHist( &hsvChannels[1], 1, 0, Mat(), s_hist, 1, &histSize, &histRange, uniform, accumulate );
        cv::calcHist( &hsvChannels[2], 1, 0, Mat(), v_hist, 1, &histSize, &histRange, uniform, accumulate );

        /// Normalize the result to [ 0, hist_ht ]
        normalize(h_hist, h_hist, 0, hist_ht, NORM_MINMAX, -1, Mat() );
        normalize(s_hist, s_hist, 0, hist_ht, NORM_MINMAX, -1, Mat() );
        normalize(v_hist, v_hist, 0, hist_ht, NORM_MINMAX, -1, Mat() );

        cv::Mat redrawMaskTop = cv::Mat::zeros(histImage.size(), CV_8U); // all 0
        redrawMaskTop( Rect(0,0,hist_wd,hist_ht*1.1) ) = 1;
        histImage.setTo( Scalar( 0, 0, 240), redrawMaskTop );

        cv::putText(histImage, "Entire Frame", cv::Point(hist_wd*0.1,hist_ht*0.08), FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,0), 2);

        for( int i = 1; i < histSize; i++ ) {
            /*
             cv::line( histImage, Point( bin_wd*(i-1), hist_ht - cvRound(h_hist.at<float>(i-1)) ) ,
             Point( bin_wd*(i), hist_ht - cvRound(h_hist.at<float>(i)) ),
             Scalar( i, meanClr[1], meanClr[2]), 2, 8, 0  );
             cv::line( histImage, Point( bin_wd*(i-1), hist_ht - cvRound(s_hist.at<float>(i-1)) ) ,
             Point( bin_wd*(i), hist_ht - cvRound(s_hist.at<float>(i)) ),
             Scalar( meanClr[0], i, meanClr[2]), 2, 8, 0  );
             cv::line( histImage, Point( bin_wd*(i-1), hist_ht - cvRound(v_hist.at<float>(i-1)) ) ,
             Point( bin_wd*(i), hist_ht - cvRound(v_hist.at<float>(i)) ),
             Scalar( meanClr[0], meanClr[1], i), 2, 8, 0  );*/
            cv::line( histImage, Point( bin_wd*(i-1), hist_ht*1.1 - cvRound(h_hist.at<float>(i-1)) ) ,
                     Point( bin_wd*(i), hist_ht*1.1 - cvRound(h_hist.at<float>(i)) ),
                     Scalar( 230/2, meanClr[1], meanClr[2]), 2, 8, 0  );
            cv::line( histImage, Point( bin_wd*(i-1), hist_ht*1.1 - cvRound(s_hist.at<float>(i-1)) ) ,
                     Point( bin_wd*(i), hist_ht*1.1 - cvRound(s_hist.at<float>(i)) ),
                     Scalar( 120/2, meanClr[1], meanClr[2]), 2, 8, 0  );
            cv::line( histImage, Point( bin_wd*(i-1), hist_ht*1.1 - cvRound(v_hist.at<float>(i-1)) ) ,
                     Point( bin_wd*(i), hist_ht*1.1 - cvRound(v_hist.at<float>(i)) ),
                     Scalar( 0/2, meanClr[1], meanClr[2]), 2, 8, 0  );
        }
#endif


        //Do this for both subtraction and redbox
        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD || currentStage == STAGE_REDBOX1) {
            fgmask = fgmask_standard;

            getLargestContour(frameDisplay, fgmask, ratio, drawColor);

            int nonzero2 = cv::countNonZero(fgmask);

            // Institute a kernel in order to remove the noise in the mask,
            // specifically removing the dots of mask in the background
            morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
            morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
        }


        Mat redboxCheckArea; //define here so that it can be used across the multiple STAGE_REDBOX1 if statements

        //Split up based on stage
        if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {


        } else if (currentStage == STAGE_REDBOX1) {
            if (sizeRefImgAlpha.data != NULL) {
                int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length


                //no point in eroding because the check part isn't actually used in the game play stage
                //cv::erode( sizeRefImgAlpha, redboxCheckArea, calibrationKernBigger );
                //cv::erode( sizeRefImgAlpha, redboxCheckArea, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelStandardWd*10, kernelStandardHt*10)));
                Mat imgAlphaKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelStandardWd*2, kernelStandardHt*2));
                cv::erode( sizeRefImgAlpha,       redboxCheckArea, imgAlphaKernel );
                cv::erode( redboxCheckArea,       redboxCheckArea, imgAlphaKernel );
                cv::erode( redboxCheckArea,       redboxCheckArea, imgAlphaKernel );
                cv::erode( redboxCheckArea,       redboxCheckArea, imgAlphaKernel );
                cv::erode( redboxCheckArea,       redboxCheckArea, imgAlphaKernel );


                /*
                redboxCheckArea = sizeRefImgAlpha.clone();
                redboxCheckArea.setTo(0);
                double centerW = redboxCheckArea.cols*0.03, centerH = redboxCheckArea.rows*0.03;
                redboxCheckArea(Rect( (redboxCheckArea.cols-centerW)/2, (redboxCheckArea.rows-centerH)/2, centerW, centerH)) = 255;*/

                //calibStartTime = -10 + 2; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds


                if (displayExtraInfoAmount > 0) {
                    Mat redboxCheckAreaDraw;
                    cv::cvtColor(redboxCheckArea, redboxCheckAreaDraw, COLOR_GRAY2RGB);
                    drawImageOptimized(frameDisplay, redboxCheckAreaDraw, frameDisplay.cols/2-redboxCheckAreaDraw.cols/2, frameDisplay.rows-redboxCheckAreaDraw.rows);
                }
                cv::resize(redboxCheckArea, redboxCheckArea, cv::Size(), ratio, ratio);
                int top = frameEditHSV.rows-redboxCheckArea.rows;
                int bottom = 0;
                int left  = (frameEditHSV.cols-redboxCheckArea.cols)/2;
                int right = frameEditHSV.cols-redboxCheckArea.cols-left;
                cv::copyMakeBorder(redboxCheckArea, redboxCheckArea, top, bottom, left, right, BORDER_CONSTANT, Scalar(0) );
                if (displayExtraInfoAmount > 0 && false) {
                    cv::rectangle(frameDisplay, cv::Point(left / ratio, top / ratio),
                                  cv::Point((redboxCheckArea.cols - right) / ratio,
                                            (redboxCheckArea.rows - bottom) / ratio), CLR_BLACK_4UC, 10);
                }


                Scalar mean, stdev;
                cv::meanStdDev(frameEditHSV, mean, stdev, redboxCheckArea);


#if (MY_OS==MAC_OS || MY_OS==LINUX_OS) && false
                Mat h_hist_check, s_hist_check, v_hist_check;
                cv::calcHist( &hsvChannels[0], 1, 0, redboxCheckArea, h_hist_check, 1, &histSize, &histRange, uniform, accumulate );
                cv::calcHist( &hsvChannels[1], 1, 0, redboxCheckArea, s_hist_check, 1, &histSize, &histRange, uniform, accumulate );
                cv::calcHist( &hsvChannels[2], 1, 0, redboxCheckArea, v_hist_check, 1, &histSize, &histRange, uniform, accumulate );

                //int maxCheck = (int) (hist_ht*cv::countNonZero(redboxCheckArea)/(redboxCheckArea*redboxCheckArea.cols));
                int maxCheck = hist_ht;
                normalize(h_hist_check, h_hist_check, 0, maxCheck, NORM_MINMAX, -1, Mat() );
                normalize(s_hist_check, s_hist_check, 0, maxCheck, NORM_MINMAX, -1, Mat() );
                normalize(v_hist_check, v_hist_check, 0, maxCheck, NORM_MINMAX, -1, Mat() );

                cv::Mat redrawMaskBottom = cv::Mat::zeros(histImage.size(), CV_8U); // all 0
                redrawMaskBottom( Rect(0,hist_ht*1.1,hist_wd,hist_ht*1.1) ) = 1;
                histImage.setTo( Scalar( 0, 0, 240), redrawMaskBottom );

                cv::putText(histImage, "Checked Area", cv::Point(hist_wd*0.1,hist_ht*1.18), FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,0), 2);

                for( int i = 1; i < histSize; i++ ) {
                    cv::line( histImage, Point( bin_wd*(i-1), hist_ht*2.2 - cvRound(h_hist_check.at<float>(i-1)) ) ,
                             Point( bin_wd*(i), hist_ht*2.2 - cvRound(h_hist_check.at<float>(i)) ),
                             Scalar( 230/2, meanClr[1], meanClr[2]), 2, 8, 0  );
                    cv::line( histImage, Point( bin_wd*(i-1), hist_ht*2.2 - cvRound(s_hist_check.at<float>(i-1)) ) ,
                             Point( bin_wd*(i), hist_ht*2.2 - cvRound(s_hist_check.at<float>(i)) ),
                             Scalar( 120/2, meanClr[1], meanClr[2]), 2, 8, 0  );
                    cv::line( histImage, Point( bin_wd*(i-1), hist_ht*2.2 - cvRound(v_hist_check.at<float>(i-1)) ) ,
                             Point( bin_wd*(i), hist_ht*2.2 - cvRound(v_hist_check.at<float>(i)) ),
                             Scalar( 0/2, meanClr[1], meanClr[2]), 2, 8, 0  );
                }

#endif

                //cout << "h avg org: " << round(mean[0]) << " ";
                //cout << "h std org: " << round(stdev[0]) << " ";



                double hue_avg, hue_stdev;
                std::tie(hue_avg, hue_stdev) = circularMeanStdDev(hsvChannels[0], 0, 180); //note hue is circular, but is from 0 to 180, NOT 0 to 360 (designed this way to be under 255)
                mean[0] = hue_avg;
                stdev[0] = hue_stdev;

                colorMean = mean;
                colorStDev = stdev;

                //cout << "avg org: " << mean << " ";
                //cout << "std org: " << stdev << ", " << endl;

                /*if (colorLowerBound[0] < 0) colorLowerBound[0] = 0;
                 if (colorLowerBound[1] < 0) colorLowerBound[1] = 0;
                 if (colorLowerBound[2] < 0) colorLowerBound[2] = 0;
                 if (colorUpperBound[0] > 255) colorUpperBound[0] = 255;
                 if (colorUpperBound[1] > 255) colorUpperBound[1] = 255;
                 if (colorUpperBound[2] > 255) colorUpperBound[2] = 255;*/
                //checkBackground(fgmask,frameDisplay,ratio);

            }



        }

        if (currentStage == STAGE_GAMEPLAY || (currentStage == STAGE_REDBOX1 && (fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694) )) {
            colorLowerBound = (colorMean - colorStDev * 1.5)-Scalar(20,20,20,0);
            colorUpperBound = (colorMean + colorStDev * 1.5)+Scalar(20,20,20,0);
            colorLowerBound[3] =   0; //alpha
            colorUpperBound[3] = 255; //alpha

            //this is "volume" in the HSV coordinate space. Thus the equation is that of a cylinder
            double volume = M_PI * (colorUpperBound[1]*colorUpperBound[1]-colorLowerBound[1]*colorLowerBound[1]) * (colorUpperBound[2]-colorLowerBound[2]) * (colorUpperBound[0]-colorLowerBound[0])/180.0;
            double volRatio = volume*1.0/(255*255*255*M_PI); //normalize so max value is 1, min value is 0
            //cout << "volume: " << volume << " " << volume/(255*255*255*M_PI) << endl;


            Mat fgmask1 = Mat(frameEditHSV.size(), CV_8UC1), fgmask2 = Mat(frameEditHSV.size(), CV_8UC1), fgmask3 = Mat(frameEditHSV.size(), CV_8UC1), fgmask_s_color, fgmask_v_color;
            if (fgLocAlg == 1650 || fgLocAlg == 1654) {
                Scalar modLowerBound = colorLowerBound;
                Scalar modUpperBound = colorUpperBound;
                //Need to add the +/-180 ranges to factor in the fact that Hue is circular (i.e. 181 is the same 1, and therefore red is split between the top and bottom of the H spectrum)
                cv::inRange(frameEditHSV, modLowerBound,                   modUpperBound,                   fgmask1);
                cv::inRange(frameEditHSV, modLowerBound-Scalar(180,0,0,0), modUpperBound-Scalar(180,0,0,0), fgmask2);
                cv::inRange(frameEditHSV, modLowerBound+Scalar(180,0,0,0), modUpperBound+Scalar(180,0,0,0), fgmask3);


            } else if (fgLocAlg == 1660 || fgLocAlg == 1664) {
                Scalar modLowerBound = cv::Scalar(colorLowerBound[0],   100,   0);
                Scalar modUpperBound = cv::Scalar(colorUpperBound[0],   255, 255);
                //Need to add the +/-180 ranges to factor in the fact that Hue is circular (i.e. 181 is the same 1, and therefore red is split between the top and bottom of the H spectrum)
                cv::inRange(frameEditHSV, modLowerBound,                   modUpperBound,                   fgmask1);
                cv::inRange(frameEditHSV, modLowerBound-Scalar(180,0,0,0), modUpperBound-Scalar(180,0,0,0), fgmask2);
                cv::inRange(frameEditHSV, modLowerBound+Scalar(180,0,0,0), modUpperBound+Scalar(180,0,0,0), fgmask3);


            } else if (fgLocAlg == 1670 || fgLocAlg == 1674 || fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694) {
                Scalar zScoreCutoffs = Scalar(2.0, 4.0, 4.0, 1.0);
                colorLowerBound = (colorMean - colorStDev.mul(zScoreCutoffs))-Scalar(5,5,5,0);
                colorUpperBound = (colorMean + colorStDev.mul(zScoreCutoffs))+Scalar(5,5,5,0);
                colorLowerBound[3] =   0; //alpha
                colorUpperBound[3] = 255; //alpha


                //In HSV, low v means darker and high s means more color (less grayscale)
                //if s->0, then v still matters, but h does not matter
                //if v->0, then it doesn't matter what h and s are
                //cout << round(cv::countNonZero(hsvChannels[2] < 50)*100.0/(frameEditHSV.cols*frameEditHSV.rows)) << " v<50, ";
                hsvChannels[0].setTo(colorMean[0], hsvChannels[2] < 50);
                hsvChannels[1].setTo(colorMean[1], hsvChannels[2] < 50);
                //cout << round(cv::countNonZero(hsvChannels[1] < 50)*100.0/(frameEditHSV.cols*frameEditHSV.rows)) << " s<50" << endl;;
                hsvChannels[0].setTo(colorMean[0], hsvChannels[1] < 50);
                cv::merge(hsvChannels, frameEditHSV);

                if (fgLocAlg == 1670 || fgLocAlg == 1674 || fgLocAlg == 1680 || fgLocAlg == 1684) {
                    Scalar modLowerBound = colorLowerBound;
                    Scalar modUpperBound = colorUpperBound;
                    //Need to add the +/-180 ranges to factor in the fact that Hue is circular (i.e. 181 is the same 1, and therefore red is split between the top and bottom of the H spectrum)
                    cv::inRange(frameEditHSV, modLowerBound,                   modUpperBound,                   fgmask1);
                    cv::inRange(frameEditHSV, modLowerBound-Scalar(180,0,0,0), modUpperBound-Scalar(180,0,0,0), fgmask2);
                    cv::inRange(frameEditHSV, modLowerBound+Scalar(180,0,0,0), modUpperBound+Scalar(180,0,0,0), fgmask3);
                } else if (fgLocAlg == 1690 || fgLocAlg == 1694) {
                    if (colorLowerBounds.size() == 0) {
                        colorLowerBounds.push_back(colorLowerBound);
                        colorUpperBounds.push_back(colorUpperBound);
                    } else {
                        colorLowerBounds[0] = (colorLowerBound);
                        colorUpperBounds[0] = (colorUpperBound);
                    }
                    //colorLowerBounds.clear();
                    //colorUpperBounds.clear();
                    //colorLowerBounds.push_back(colorLowerBound);
                    //colorUpperBounds.push_back(colorUpperBound);


                    for (int i = 0; i < tapLocationsProcessed.size(); i++) {
                        if (!tapLocationsProcessed[i]) {
                            //Scalar newLower = Scalar(0,0,0,0);
                            //Scalar newUpper = Scalar(255,255,255,255);
                            cv::Vec4b &pixel = frameEditHSV.at<cv::Vec4b>(tapLocationYs[i]*ratio/4, tapLocationXs[i]*ratio/4);
                            Scalar newLower = Scalar(pixel[0],pixel[1],pixel[2],255)-5*Scalar(10,10,10,0);
                            Scalar newUpper = Scalar(pixel[0],pixel[1],pixel[2],255)+5*Scalar(10,10,10,0);
                            if (colorLowerBounds.size() > i+1) {
                                colorLowerBounds[i+1] = newLower;
                                colorUpperBounds[i+1] = newUpper;
                            } else if (colorLowerBounds.size() > i+1-1) {
                                colorLowerBounds.push_back(newLower);
                                colorUpperBounds.push_back(newUpper);
                            } else throw std::exception();
                            tapLocationsProcessed[i] = true;
                        }
                    }


                    //fgmask1 = Mat(frameEditHSV.size(), CV_8UC1); //set default values for fgmaskN so the bitwise_and doesn't throw an error within the loop
                    //fgmask2 = Mat(frameEditHSV.size(), CV_8UC1);
                    //fgmask3 = Mat(frameEditHSV.size(), CV_8UC1);
                    fgmask1.setTo(0);
                    fgmask2.setTo(0);
                    fgmask3.setTo(0);
                    for (int i = 0; i < colorLowerBounds.size(); i++) {
                        Scalar modLowerBound = colorLowerBounds[i];
                        Scalar modUpperBound = colorUpperBounds[i];
                        Mat newFgmask1, newFgmask2, newFgmask3;
                        //Need to add the +/-180 ranges to factor in the fact that Hue is circular (i.e. 181 is the same 1, and therefore red is split between the top and bottom of the H spectrum)
                        cv::inRange(frameEditHSV, modLowerBound,                   modUpperBound,                   newFgmask1);
                        cv::inRange(frameEditHSV, modLowerBound-Scalar(180,0,0,0), modUpperBound-Scalar(180,0,0,0), newFgmask2);
                        cv::inRange(frameEditHSV, modLowerBound+Scalar(180,0,0,0), modUpperBound+Scalar(180,0,0,0), newFgmask3);
                        cv::bitwise_or(fgmask1, newFgmask1, fgmask1);
                        cv::bitwise_or(fgmask2, newFgmask2, fgmask2);
                        cv::bitwise_or(fgmask3, newFgmask3, fgmask3);
                    }
                }
            }


            // | is the logical OR operator, and & is the logical AND operator
            if (fgLocAlg == 1650 || fgLocAlg == 1660 || fgLocAlg == 1670 || fgLocAlg == 1680 || fgLocAlg == 1690) {
                //cv::bitwise_or(fgmask1, fgmask2, fgmask);
                //cv::bitwise_or(fgmask, fgmask3, fgmask);
                fgmask = (fgmask1 | fgmask2 | fgmask3);
            } else if (fgLocAlg == 1654 || fgLocAlg == 1664 || fgLocAlg == 1674 || fgLocAlg == 1684 || fgLocAlg == 1694) {
                //cv::bitwise_or(fgmask1, fgmask2, fgmask);
                //cv::bitwise_or(fgmask, fgmask3, fgmask);
                //cv::bitwise_and(fgmask, fgmask_standard, fgmask);
                fgmask = (fgmask1 | fgmask2 | fgmask3) & fgmask_standard;
            }


            if ( currentStage == STAGE_REDBOX1 && (fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694) && sizeRefImgAlpha.data != NULL ) {
                //remember that at this point, fgmask isn't the actual hand. Instead, it is a "perception of the hand" which is most likely to be wrong because the user might not
                //have put their hand on the screen yet or put it in the right location. Thus the wrong color scheme is used and a completely different part of the image that doesn't look like the hand is used.
                int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

                int nonzero2 = cv::countNonZero(fgmask);
                //Check the center of fgmask to see if it is in the redbox area
                //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
                Moments m = moments(fgmask,true);
                Point p(m.m10/m.m00, m.m01/m.m00);
                /*
                bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
                bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
                bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
                bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;
                //bool isCenterCorrect = redboxCheckArea.data != NULL ? redboxCheckArea.data<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

                Mat sizeRefImgAlphaSmall;
                cv::resize(sizeRefImgAlpha, sizeRefImgAlphaSmall, cv::Size(), ratio, ratio);
                int intersection = cv::countNonZero(fgmask & sizeRefImgAlphaSmall);
                double percentCovered = nonzeroSizeRef==0 ? 0 : intersection*1.0/nonzeroSizeRef; //percent of the reference hand covered by the perceived hand

                //On Sept 8th, 2019, I (Rahul) added the abs(.) because I think it was a bug that had somehow gone unnoticed for a while... ?. I added it each time this type of test came up (total of 4 matches, including the 4th one I added that day for alg 3000).
                bool resetCondition = (nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > percentAreaOverlapToFinishRedBox || !isCenterCorrect || percentCovered < 0.89;


                if ( resetCondition ) { //checks if hand is significantly bigger than reference hand
                    calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
                } else if ( (fgLocAlg == 6 || fgLocAlg == 8) && (nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef < 0) { //checks if the hand is smaller than the reference
                    calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
                } else {
                    useGreenTint = 2;
                }

            }


            //Don't do the morphologyEx because it removes the fingers when they are two n arrow
            // Institute a kernel in order to remove the noise in the mask,
            // specifically removing the dots of mask in the background
            //morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
            //morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
            //cv::dilate( fgmask_MOG, fgmask_MOG_dilated, calibrationKernBigger );


            if (displayExtraInfoAmount < 1) {
                if (fgLocAlg == 1670 || fgLocAlg == 1674 || fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694) {
                    //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio, drawColor); //make 0.03 instead of the default 0.01 (indicated by passing -1) because we just did a dilation
                    //cv::dilate( fgmask, fgmask, calibrationKern );

                    Mat calibrationKernBig = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(0.5+5/ratio, 0.5+5/ratio));
                    //fgmask = getAllLargeContours(frameDisplay, fgmask, -1, ratio, drawColor);
                    fgmask = getAllLargeContoursWithKernelOperations(frameDisplay, fgmask, -1, ratio, drawColor, calibrationKern, calibrationKernBig);

                } else {
                    getLargestContour(frameDisplay, fgmask, ratio, drawColor);

                    //this is done to get the outline of the fingers/hands which might not be caught because they are slightly different color
                    //this makes the color lines slightly within the mask. This can't easily be avoided because if we put this line
                    //inside getLargestContour before drawing the lines, you'd also have to somehow dilate the contour too
                    cv::dilate( fgmask, fgmask, calibrationKern );
                }
            }
            //getLargestContour(frameDisplay, fgmask, ratio, drawColor);

        }


#if (MY_OS==MAC_OS || MY_OS==LINUX_OS) && false
        Mat histImageDispl;
        cv::cvtColor(histImage, histImageDispl, COLOR_HSV2BGR);
        imshow("calcHist Demo", histImageDispl );
#endif


    } else if (fgLocAlg == 1700) {

        if (currentStage == STAGE_GAMEPLAY) {
            fgmask = Mat::ones(frameEdit.size(), CV_8UC1)*255;;
            cv::cvtColor(frameDisplay, frameDisplay, COLOR_RGBA2GRAY);

            cv::Canny(frameDisplay, frameDisplay, 50, 250);
            //int iterations = 5;
            //cv::dilate(frameDisplay, frameDisplay, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelStandardWd*2, kernelStandardHt*2)), cv::Point(-1,-1), iterations );

            cv::distanceTransform(~frameDisplay, frameDisplay, DIST_L1, 3);
            cv::threshold(frameDisplay, frameDisplay, 25, -1, THRESH_TRUNC);
            frameDisplay = frameDisplay * 10;
            double minVal;
            double maxVal;
            cv::Point minLoc;
            cv::Point maxLoc;
            cv::minMaxLoc(~frameDisplay , &minVal, &maxVal, &minLoc, &maxLoc);
            //cv::normalize(frameDisplay, frameDisplay, 0, 255, cv::NORM_MINMAX);
            cout << "type: " << frameDisplay.type() << ", from " << minVal << " (at " << minLoc << ") to " << maxVal << " (at " << minLoc << ")" << endl;
            frameDisplay.convertTo(frameDisplay, CV_8UC1);



            cv::cvtColor(frameDisplay, frameDisplay, COLOR_GRAY2RGBA);

        } else {
            cv::GaussianBlur( frameEdit, frameEdit, Size( 5, 5 ), 0, 0 );
            fgbg->apply(frameEdit, fgmask, learning_rate); // apply background subtractor

        }

    } else if (fgLocAlg >= 3000 && fgLocAlg < 5000) {
        frameCounter++;

        if (frameCounter % segmentationFrameDelayAmt == 0) {
            //Example: OpenCV's samples/dnn/segmentation.cpp, URL: https://docs.opencv.org/master/d4/d88/samples_2dnn_2segmentation_8cpp-example.html
            Mat blob, frameEdit3C;
            cv::cvtColor(frameEdit, frameEdit3C, COLOR_RGBA2RGB);
            //dnn::blobFromImage(frame, blob, scale, Size(inpWidth, inpHeight), mean, swapRB, false);
            double rat = 1;
            if (frameEdit.cols < frameEdit.rows) { //portrait
                cv::resize(frameEdit3C, frameEdit3C, cv::Size(96*rat,128*rat), cv::INTER_NEAREST);
            } else {
                //When RY tried (127,96), it didn't crash, but (126,96),(125,96),(122,96),(121,96),(120,96), and even larger sizes like (129,96) and (140,96) all crashed.
                cv::resize(frameEdit3C, frameEdit3C, cv::Size(128*rat,96*rat), cv::INTER_NEAREST);
            }
            dnn::blobFromImage(frameEdit3C, blob);
            net.setInput(blob);
            Mat score = net.forward();
            colorizeSegmentation(score, fgmask);
            //fgmask = cv::Mat(120, 90, CV_8U, double(0));
            cv::resize(fgmask, fgmask, cv::Size(frameEdit.cols,frameEdit.rows), cv::INTER_NEAREST);


            if (displayExtraInfoAmount <= 1) {
                // Institute a kernel in order to remove the noise in the mask,
                // specifically removing the dots of mask in the background
                morphologyEx(fgmask, fgmask, MORPH_OPEN,  calibrationKern);
                morphologyEx(fgmask, fgmask, MORPH_CLOSE, calibrationKern);
            }


            oldFgMask = fgmask;

        } else {
            fgmask = oldFgMask.clone();
        }

        int nonzero2 = cv::countNonZero(fgmask);

        if (displayExtraInfoAmount <= 1) {
            //This part is done every frame, even when masked is copied over, because drawColor might change
            getLargestContour(frameDisplay, fgmask, ratio, drawColor);
        }

        int nonzero3 = cv::countNonZero(fgmask);

        //Redbox comparison code copied from algorithm 4
        const double percentAreaOverlapToFinishRedBox = 0.6; //defined as the difference in area divided by the redbox hand size.
        const int secondsOfRedBoxNeedToHold = 1;

        if (currentStage == STAGE_BG_SUBTRACT_STANDARD) {
            goToRedBoxStage();

        } else if (currentStage == STAGE_REDBOX1 && sizeRefImgAlpha.data != NULL) {
            int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

            //Check the center of fgmask to see if it is in the redbox area
            //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
            Moments m = moments(fgmask,true);
            Point p(m.m10/m.m00, m.m01/m.m00);
            /*
             bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
             bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
             bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
            bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

            //On Sept 8th, 2019, I (Rahul) added the abs(.) because I think it was a bug that had somehow gone unnoticed for a while... ?. I added it each time this type of test came up (total of 4 matches, including the 4th one I added that day for alg 3000).
            if ( abs(nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > percentAreaOverlapToFinishRedBox || !isCenterCorrect) { //checks if hand is significantly bigger than reference hand
                calibStartTime = -10 + secondsOfRedBoxNeedToHold; //if not matching, then restart counter to hold for secondsOfBGNeedToRemoveBody more seconds
            } else {
                useGreenTint = 2;
            }

            Mat redboxCheckArea;
            //no point in eroding because the check part isn't actually used in the game play stage
            //cv::erode( sizeRefImgAlpha, redboxCheckArea, calibrationKernBigger );
            redboxCheckArea = sizeRefImgAlpha.clone();
            //redboxCheckArea.setTo(200);
            if (displayExtraInfoAmount > 0) {
                Mat redboxCheckAreaDraw;
                cv::cvtColor(redboxCheckArea, redboxCheckAreaDraw, COLOR_GRAY2RGB);
                drawImageOptimized(frameDisplay, redboxCheckAreaDraw, frameDisplay.cols/2-redboxCheckAreaDraw.cols/2, frameDisplay.rows-redboxCheckAreaDraw.rows);
            }
            cv::resize(redboxCheckArea, redboxCheckArea, cv::Size(), ratio, ratio);
            int top = frameEdit.rows-redboxCheckArea.rows;
            int bottom = 0;
            int left  = (frameEdit.cols-redboxCheckArea.cols)/2;
            int right = frameEdit.cols-redboxCheckArea.cols-left;
            cv::copyMakeBorder(redboxCheckArea, redboxCheckArea, top, bottom, left, right, BORDER_CONSTANT, Scalar(0) );

            //checkBackground(fgmask,frameDisplay,ratio);
        }



    } else {
        cout << "fgLocAlg value of " + std::to_string(fgLocAlg) + " is invalid value." << endl;
        //throw std::runtime_error("can't find algorithm " + std::to_string(fgLocAlg));
        throw std::invalid_argument("can't find algorithm " + std::to_string(fgLocAlg));
    }

    if (currentStage == STAGE_REDBOX1 || currentStage == STAGE_GAMEPLAY) {
        if (fgLocAlg < 1600 || fgLocAlg >= 1700) {
            checkBackground(fgmask,frameDisplay,ratio);
        }
    }


    //output is into frame_with_mask.
    //frame_with_mask(i)=frameEdit(i) and frameEdit(i) if fgmask(i)!=0

    //return contour;//fgmask;
}


string type2str(int type) {
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}
// input: Score is the output of net.forward, and is a blob.
void colorizeSegmentation(const Mat &score, Mat &segm) {
    // Score comes in dimensions like 1x2x120x90(1) where parenthesis is the channel ct and the 120 and 90 depend on the image size
    // First dimension is batch size (which will always be 1 in this case because we are testing not training)
    // Second dimension represents a likelihood measure of that pixel being in each of the classes
    // for example: if just detecting background/foreground, it would be likelihood of  being the background
    // (zeroeth element) and of that pixel being the foreground (first/last element; total length=2)
    // Last two dimensions are just the image size

    //First flatten (Convert 1x2x120x90(1) -> 2x120x90(1))
    const int flattenedSize1[3] = {score.size[1],score.size[2],score.size[3]}; //skips subtracted.size[0] because that is the dimensional being flattened bc its length is 1
    Mat flattenedScore1 = score.reshape(1, 3, flattenedSize1);

    // ############ old implementation when only had two classes ############
     /*Mat fgOrBg;// = (flattenedScore1.row(0) < flattenedScore1.row(1));
     __android_log_print(ANDROID_LOG_ERROR, "SCORE SIZE", "%d %d %d %d",flattenedScore1.row(0).rows,
                         flattenedScore1.row(1).rows, flattenedScore1.row(2).rows, flattenedScore1.row(0).cols);
     bitwise_or(flattenedScore1.row(0) < flattenedScore1.row(1),flattenedScore1.row(0) < flattenedScore1.row(2) , fgOrBg);
    //Flatten again to get 2D binary mask (Convert 1x120x90(1) -> 120x90(1))
    const int flattenedSize2[2] = {fgOrBg.size[1],fgOrBg.size[2]}; //skips subtracted.size[0] because that is the dimensional being flattened bc its length is 1
    segm = fgOrBg.reshape(1, 2, flattenedSize2);*/
    //Get argmax (i.e. figure out whether foreground likelihood or background likelihood)
    /*Mat fgOrBg = flattenedScore1.row(0) < flattenedScore1.row(1);

    //Flatten again to get 2D binary mask (Convert 1x120x90(1) -> 120x90(1))
    const int flattenedSize2[2] = {fgOrBg.size[1],fgOrBg.size[2]}; //skips subtracted.size[0] because that is the dimensional being flattened bc its length is 1
    segm = fgOrBg.reshape(1, 2, flattenedSize2);
    */
    /*segm = cv::Mat::zeros(cv::Size(score.size[3],score.size[2]), CV_8UC1);

    int i,j;
    for (i = 0; i < score.size[2]; i++) {
        for (j = 0; j < score.size[3]; j++) {
            double maxval = flattenedScore1.at<double>(0,i,j);
            for (int channel = 1; channel < score.size[1]; channel++) {
                if (flattenedScore1.at<uchar>(channel,i,j) > maxval) {
                    maxval = flattenedScore1.at<uchar>(channel,i,j);
                    segm.at<uchar>(i,j) = 255;//channel;
                }
            }
        }
    }*/

    Mat fgOrBg = flattenedScore1.row(0) < flattenedScore1.row(1);

    //Flatten again to get 2D binary mask (Convert 1x120x90(1) -> 120x90(1))
    const int flattenedSize2[2] = {fgOrBg.size[1],fgOrBg.size[2]}; //skips subtracted.size[0] because that is the dimensional being flattened bc its length is 1
    segm = fgOrBg.reshape(1, 2, flattenedSize2);
}

//Source: https://stackoverflow.com/questions/33041900/opencv-threshold-with-mask
double otsu_8u_with_mask(const Mat1b src, const Mat1b& mask) {
    const int N = 256;
    int M = 0;
    int i, j, h[N] = { 0 };
    for (i = 0; i < src.rows; i++) {
        const uchar* psrc = src.ptr(i);
        const uchar* pmask = mask.ptr(i);
        for (j = 0; j < src.cols; j++) {
            if (pmask[j]) {
                h[psrc[j]]++;
                ++M;
            }
        }
    }

    double mu = 0, scale = 1. / (M);
    for (i = 0; i < N; i++) {
        mu += i*(double)h[i];
    }

    mu *= scale;
    double mu1 = 0, q1 = 0;
    double max_sigma = 0, max_val = 0;

    for (i = 0; i < N; i++) {
        double p_i, q2, mu2, sigma;

        p_i = h[i] * scale;
        mu1 *= q1;
        q1 += p_i;
        q2 = 1. - q1;

        if (std::min(q1, q2) < FLT_EPSILON || std::max(q1, q2) > 1. - FLT_EPSILON) {
            continue;
        }

        mu1 = (mu1 + i*p_i) / q1;
        mu2 = (mu - q1*mu1) / q2;
        sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);
        if (sigma > max_sigma) {
            max_sigma = sigma;
            max_val = i;
        }
    }

    return max_val;
}
//Does the same as cv::threshold but can take a mask argument
double threshold_with_mask(Mat1b& src, Mat1b& dst, double thresh, double maxval, int type, const Mat1b& mask) {
    if (mask.empty() || (mask.rows == src.rows && mask.cols == src.cols && countNonZero(mask) == src.rows * src.cols)) {
        // If empty mask, or all-white mask, use cv::threshold
        thresh = cv::threshold(src, dst, thresh, maxval, type);
    } else {
        // Use mask
        bool use_otsu = (type & THRESH_OTSU) != 0;
        if (use_otsu) {
            // If OTSU, get thresh value on mask only
            double thresh42 = otsu_8u_with_mask(src, mask);
            if (fgLocAlg==43) {
                double thresh43, separation43;
                std::tie(thresh43, separation43) = otsu_8u_with_mask_returning_separation(src, mask);
                thresh = thresh43;
                cout << thresh42-thresh43 << " = " << thresh42 << " - " << thresh43 << " = #42 - #43"  << endl;
            } else {
                thresh = thresh42;
            }

            // Remove THRESH_OTSU from type
            type &= THRESH_MASK;
        }

        // Apply cv::threshold on all image
        thresh = cv::threshold(src, dst, thresh, maxval, type);

        // Copy original image on inverted mask
        src.copyTo(dst, ~mask);
    }
    return thresh;
}

//For equations see "How Otsu’s Binarization Works?" in https://docs.opencv.org/3.0-beta/doc/py_tutorials/py_imgproc/py_thresholding/py_thresholding.html
//double threshold_mod_OTSU(Mat1b& src, Mat1b& dst, const Mat1b& mask) {
std::tuple<double, double> otsu_8u_with_mask_returning_separation(const Mat1b src, const Mat1b& mask) {
    //STEP 1: Create the histogram
    const int N = 256; //number of possible values of the image (8 bit image so 256)
    int hist[N] = { }; //hist (h) represents the histogram. Will have N values.
    int M = 0; //Total number of pixels to process (only those within the mask). Also, equal to the sum of hist.
    for (int row_it = 0; row_it < src.rows; row_it++) {
        const uchar* psrc_row  =  src.ptr(row_it);
        const uchar* pmask_row = mask.ptr(row_it);
        for (int col_it = 0; col_it < src.cols; col_it++) {
            if (pmask_row[col_it]) {
                hist[psrc_row[col_it]]++;
                M++;
            }
        }
    }
    double scale = 1. / (M); //scale is the ratio (<1) for converting hist from absolute units to a probability distribution

    int thresh_opt;
    double mu = 0;
    for (thresh_opt = 0; thresh_opt < N; thresh_opt++) {
        mu += thresh_opt*(double)hist[thresh_opt];
    }

    mu *= scale;

    double mu1 = 0; //mu1 is the mean of class 1 (define here to simplify calculation)
    double q1 = 0; //q1 is the proportion of class 1 (define here to simplify calculation)
    double max_sigma = -10000, max_thresh = 0; //max_thresh is maximizing threshold of the sigma function

    for (thresh_opt = 0; thresh_opt < N; thresh_opt++) { //iterate through all possible thresholds and find out which minimized the goal function, sigma
        double q2, mu2, sigma;
        double p_i = hist[thresh_opt] * scale; //p_i represents p(i), the histogram value of i in the probability space

        //double mu1_q1 = mu1*q1; //This is not the actual value of mu1 yet. Equation will be finished later.
        q1 += p_i; //q1 is the proportion of class 1; uses old q1 as a shortcut
        q2 = 1. - q1; //q2 is the proportion of class 2

        if (std::min(q1, q2) < FLT_EPSILON || std::max(q1, q2) > 1. - FLT_EPSILON) {
            //ignore thresholds where all pixels would be in one class
            continue;
        }

        mu1 = (mu1*(q1-p_i) + thresh_opt*p_i) / q1; //uses the old mu1, multiplied by old q1, as a shortcut (like q1 does)
        mu2 = (mu  - q1 *mu1) / q2;
        sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);
        if (sigma > max_sigma) {
            max_sigma  = sigma;
            max_thresh = thresh_opt;
        }
    }

    double separation = max_sigma;

    return std::make_tuple(max_thresh, separation);
}

double threshold_mod_OTSU(Mat1b& src, Mat1b& dst, const Mat1b& mask) {
    double thresh, separation;
    std::tie(thresh, separation) = otsu_8u_with_mask_returning_separation(src, mask);
    thresh = cv::threshold(src, dst, thresh, 255, THRESH_BINARY);
    src.copyTo(dst, ~mask);
    return thresh;
}

//static const int* getFontData(int fontFace) {
    // assume always HERHSEY SIMPLEX FONT
    /*const int HersheySimplex[] = {
            (9 + 12*16) + (16<<8),
            2199, 714, 717, 733, 719, 697, 734, 716, 721, 722, 728, 725, 711, 724, 710, 720,
            700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 712, 713, 691, 726, 692,
            715, 690, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513,
            514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 693, 584,
            694, 2247, 586, 2249, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611,
            612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626,
            695, 723, 696, 2246 };*/
    //WARNING: "Address of stack memory associated with local variable 'HersheySimplex' returned"
    //return HersheySimplex;
    /*
      bool isItalic = (fontFace & FONT_ITALIC) != 0;
     const int* ascii = 0;

    switch( fontFace & 15 )
    {
        case FONT_HERSHEY_SIMPLEX:
            ascii = HersheySimplex;
            break;
        case FONT_HERSHEY_PLAIN:
            ascii = !isItalic ? HersheyPlain : HersheyPlainItalic;
            break;
        case FONT_HERSHEY_DUPLEX:
            ascii = HersheyDuplex;
            break;
        case FONT_HERSHEY_COMPLEX:
            ascii = !isItalic ? HersheyComplex : HersheyComplexItalic;
            break;
        case FONT_HERSHEY_TRIPLEX:
            ascii = !isItalic ? HersheyTriplex : HersheyTriplexItalic;
            break;
        case FONT_HERSHEY_COMPLEX_SMALL:
            ascii = !isItalic ? HersheyComplexSmall : HersheyComplexSmallItalic;
            break;
        case FONT_HERSHEY_SCRIPT_SIMPLEX:
            ascii = HersheyScriptSimplex;
            break;
        case FONT_HERSHEY_SCRIPT_COMPLEX:
            ascii = HersheyScriptComplex;
            break;
        default:
            CV_Error( CV_StsOutOfRange, "Unknown font type" );
    }
    return ascii;*/
//}

/*double getTextScaleFromHeight(int fontFace, int thickness, int height) {
    Size size;
    double view_x = 0;
    //const char **faces = g_HersheyGlyphs;
    const int* ascii = getFontData(fontFace);

    int base_line = (ascii[0] & 15);
    int cap_line = (ascii[0] >> 4) & 15;

    double fontScale = static_cast<double>(height - static_cast<double>((thickness + 1)) / 2.0) / static_cast<double>(cap_line + base_line);

    return fontScale;
}*/

double angleBetween(const Point& v1, const Point& v2) {
    double cosAngle = v1.dot(v2) / (cv::norm(v1) * cv::norm(v2));
    if (cosAngle > 1.0)
        return 0.0;
    else if (cosAngle < -1.0)
        return CV_PI;
    return std::acos(cosAngle);
}

//0 degrees is rightward, 90 degrees is downward (increasing y)
void drawDashedLine(cv::Mat &drawOn, cv::Point startPt, cv::Point endPt, double dashPercentage, int dashCt, const Scalar& color, int thickness) {
    int spaceCt = dashCt-1; //number of non-dashes
    int segmentCt = spaceCt+dashCt; //number of dashes or non-dashes
    double totLength = cv::norm(endPt-startPt);
    //Knowns: totLength, dashCt, spaceCt, dashPercentage
    //Unknowns: dashLength, spaceLength
    //1) dashLength*dashCt + spaceLength*spaceCt == totLength;
    //2) dashLength/(dashLength+spaceLength) == dashPercentage
    //Manipulating the 2) equation
    //dashLength - dashLength*dashPercentage ==  dashPercentage*spaceLength
    //dashLength ==  dashPercentage*spaceLength/(1 - dashPercentage)
    //Plug into 1)
    // dashPercentage*spaceLength/(1 - dashPercentage)*dashCt + spaceLength*spaceCt == totLength
    // spaceLength*(dashPercentage/(1 - dashPercentage)*dashCt + spaceCt) == totLength
    // spaceLength == totLength / (dashPercentage/(1 - dashPercentage)*dashCt + spaceCt)
    // spaceLength == totLength / (dashCt*dashPercentage/(1 - dashPercentage) + spaceCt)
    // dashLength ==  dashPercentage/(1 - dashPercentage) * totLength / (dashCt*dashPercentage/(1 - dashPercentage) + spaceCt)
    double spaceLength = totLength / (dashCt*dashPercentage/(1 - dashPercentage) + spaceCt);
    double dashLength =  dashPercentage/(1 - dashPercentage) * totLength / (dashCt*dashPercentage/(1 - dashPercentage) + spaceCt);
    double degrees = 180/M_PI* atan2(endPt.y-startPt.y, endPt.x-startPt.x); //y before x
    drawDashes(drawOn, startPt, degrees, dashLength, spaceLength, dashCt, color, thickness);
}

void drawDashes(cv::Mat &drawOn, cv::Point startPt, double degrees, double dashLength, double spaceLength, int dashCt, const Scalar &color, int thickness) {
    double radians = degrees*M_PI/180;
    cv::Point normVectorAsPt = cv::Point( cos(radians), sin(radians) );
    for (int i= 0; i < dashCt; i++) {
        Point endPt = startPt + normVectorAsPt*dashLength;
        cv::line(drawOn, startPt, endPt, color, thickness);
        startPt = endPt + normVectorAsPt*spaceLength;
        //cv::line(caterpillarGameScenery, cv::Point(limit_loc,markStart), cv::Point(limit_loc,distance*4.5), markLineClr, distance/8);
        //cv::line(caterpillarGameScenery, cv::Point(limit_loc,markStart), cv::Point(limit_loc,distance*4.5), outlineClr, distance/18);
    }
}

double scaleToThicknessRatio = 1.7;
int getThicknessFromFontScale(double fontScale) {
    return (int) (scaleToThicknessRatio*fontScale+1.0); //+1.0 to round up to nearest int
}
double getFontScaleFromThickness(double thickness) {
    return (int) (thickness/scaleToThicknessRatio+1.0); //+1.0 to round up to nearest int
}

void applyColorMapKeepingAlpha(cv::Mat& picture, int map) {
    Mat pictureRGB, alphaCh;
    cv::extractChannel(picture, alphaCh, 3); //extract the 4th channel (aka index 3)
    cv::cvtColor(picture, pictureRGB, COLOR_RGBA2RGB);
    cv::applyColorMap(pictureRGB, pictureRGB, map);
    Mat tmp[] = {pictureRGB, alphaCh};
    cv::merge(tmp, 2, picture);
}

// writes the text on the image only in the rectangle writing area
// fills whole rectangle with s array
// takes in vector where each element is the text to write and the color to write it in
double writeText_onRect(Mat& img, std::vector<tuple<string,cv::Scalar>> ss) {
    int baseline=0;
    int thickness;
    // amountToAdd is the distance between the top of the screen
    // and the text so it doesn't start exactly from the top but a little below
    int amountToAdd;
    if (img.cols < img.rows) { //portrait
        thickness = 5; //temporary
        amountToAdd = textWriteRectangle.height*0.2;

    } else { //landscape
        thickness = 5; //temporary
        amountToAdd = textWriteRectangle.height*0.05;
    }
    Size standardTextSize;
    double fontScale = -1;

    string longest = get<0>(ss.front());
    for (vector< tuple<string,cv::Scalar> > ::const_iterator i = ss.begin(); i != ss.end(); ++i) {
        if (get<0>(*i).length() > longest.length()) {
            longest = get<0>(*i);
        }
    }
    // size for first member of s when font size = 1
    standardTextSize = cv::getTextSize(longest, cv::FONT_HERSHEY_SIMPLEX, 1, thickness,
                                       &baseline);
    fontScale = double(textWriteRectangle.height) /
                double(standardTextSize.height) /
                double(ss.size());
    thickness = getThicknessFromFontScale(fontScale);
    Size textSize = cv::getTextSize(longest,cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
    if (textSize.width > textWriteRectangle.width) {
        fontScale = fontScale*double(textWriteRectangle.width)/double(textSize.width);
        thickness = getThicknessFromFontScale(fontScale);
    }
    standardTextSize = cv::getTextSize(longest, cv::FONT_HERSHEY_SIMPLEX, fontScale,
                                       thickness, &baseline);


    double currentSS = 1; //represents the spacing amount (i.e. value of 1 means single spaced.
    int ct = 0;
    for (vector< tuple<string,cv::Scalar> > ::const_iterator i = ss.begin(); i != ss.end(); ++i) {
        ct++;
        string s = get<0>(*i);
        Size textSize = cv::getTextSize(s,cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
        int spacing_offset = 1*(currentSS)*standardTextSize.height; //a multiplier of one in the front means double space
        // old way
        // int y_pos = amountToAdd = standardTextSize.height * currentSS + spacing_offset ;
        //int y_pos=img.rows/2 - (ss.size()*(standardTextSize.height +amountToAdd))/2 + standardTextSize.height * currentSS+amountToAdd + spacing_offset ;
        int y_pos= standardTextSize.height * currentSS+amountToAdd;// + spacing_offset ;
        if (img.rows < img.cols) { // landscape
            y_pos += spacing_offset;
        } else if (ct>1 && ss.size() <= 2) {
            y_pos += spacing_offset*0.25;
        }
        int x_pos = (textWriteRectangle.width - textSize.width)/2;
        if (x_pos < 0) {
            x_pos = 0;
        }
        cv::Point writePoint(x_pos,y_pos);
        /*if (currentSS ==1) { // first one
            y_pos+=amountToAdd;
        }*/
        /*if (textSize.width > textWriteRectangle.width) {
            double tempFontScale = fontScale*double(textWriteRectangle.width)/double(textSize.width);
            if (shouldWriteOnIm) {

                cv::putText(img, s, writePoint, cv::FONT_HERSHEY_SIMPLEX,
                            tempFontScale,
                            cv::Scalar(0, 0, 0, 255), thickness + 4); //black outside
                cv::putText(img, s, writePoint, cv::FONT_HERSHEY_SIMPLEX,
                            tempFontScale, get<1>(*i), thickness);
            } else {
                bufferForWriting.push_back( tuple<string,double, cv::Point,cv::Scalar>(s,tempFontScale,writePoint,get<1>(*i)) );
            }
        } else {*/
            if (shouldWriteOnIm) {
                // multiply by 0.85 to give some room between text
                cv::putText(img, s, writePoint, fontType,
                            fontScale, CLR_BLACK_4UC, thickness*3); //black outside
                cv::putText(img, s, writePoint, fontType,
                            fontScale, get<1>(*i), thickness);
            } else {
                bufferForWriting.push_back( tuple<string, double, cv::Point,cv::Scalar>(s,fontScale,writePoint,get<1>(*i)) );
            }
        //}
        currentSS++;
    }
    return fontScale;
}

// writes the text on the image - loc and color are optional, will default to placing in the top and red
double writeText(Mat& img, string s, int xloc, int yloc, double fontScale, cv::Scalar color) {
    int thickness;
    if (img.cols < img.rows) { //portrait
        thickness = fontScale < 0 ? 5 : getThicknessFromFontScale(fontScale);
    } else { //landscape
        thickness = fontScale < 0 ? 5 : getThicknessFromFontScale(fontScale);
    }
    int baseline=0;
    if (fontScale < 0) { // fontScale not set
        //fontScale = s.length()*(-0.05) + 2.75;//(img.cols-loc.x-10)/260.0;
        /*int height = int(img.rows/s.length()/2.0);
        if (height > img.rows/20.0) height = int(img.rows/20.0);
        fontScale =  getTextScaleFromHeight(cv::FONT_HERSHEY_SIMPLEX, thickness, height);*/

        Size textSize = cv::getTextSize(s,cv::FONT_HERSHEY_SIMPLEX,1, thickness, &baseline);
        fontScale = double(img.cols)/textSize.width *0.9;
        if (fontScale > 5) fontScale = 5;
        thickness = getThicknessFromFontScale(fontScale);
    }
    Size textSize = cv::getTextSize(s,cv::FONT_HERSHEY_SIMPLEX,fontScale, thickness, &baseline);
    if (xloc == -1) { // xloc undefined
        xloc = (img.cols-textSize.width)/2;//int(img.cols * 0.05);
    }
    if (yloc == -1) { // yloc undefined
        if (stackedUpText) {
            yloc = last_y + int(textSize.height*1.25);
        } else {
            yloc = int(img.rows * 0.20);
        }
    }

    cv::Point loc(xloc,yloc);
    if (textSize.height > loc.y) loc.y = textSize.height*1.1;
    //cv::rectangle(img, cv::Rect(xloc,yloc,textSize.width,textSize.height), cv::Scalar( 250, 250, 250,1), 40, 8);
    /*if (countNonZero(fgmask) == 0) {
        cv::putText(img,s, loc, cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(0,0,0,255), thickness+4); //black outside
        cv::putText(img,s, loc, cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness);
    } else {
        Mat writtenIm = Mat::zeros(img.size(), img.type());
        cv::putText(writtenIm, s, loc, cv::FONT_HERSHEY_SIMPLEX, fontScale,
                    cv::Scalar(0, 0, 0, 255), thickness + 4); //black outside
        cv::putText(writtenIm, s, loc, cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness);
        last_y = loc.y;

        for (int y = 0; y < loc.y; ++y) {
            for (int x = 0; x < loc.x; ++x) {
                if (writtenIm.at<cv::Scalar>(y, x) != cv::Scalar(0, 0, 0)) {
                    img.at<cv::Scalar>(y, x) = cv::Scalar(255, 0, 0, 255);
                }
            }
        }
    }*/

    cv::putText(img,s, loc, fontType, fontScale, CLR_BLACK_4UC, thickness*3); //black outline
    cv::putText(img,s, loc, fontType, fontScale, color, thickness);

    return fontScale;
}

/*
void ensureHandAtTopLevel(Mat& frameDisplay, Mat& fgmask, Mat& originalIm) {
    Mat foreground = Mat::zeros(originalIm.size(), originalIm.type());
    Mat background = Mat::zeros(originalIm.size(), originalIm.type());
    bitwise_and(originalIm, originalIm, foreground, fgmask); //output into foreground
    Mat bgmask;
    bitwise_not(fgmask,bgmask);
    bitwise_and(frameDisplay, frameDisplay, background, bgmask); //alternatively use Scalar::all(255)-fgmask instead of bgmask

    double alpha = 1; //increase contrast if this is greater than 1
    double beta = -127*(alpha-1); //-127*(alpha-1) is enough to offset contrast, anymore/less will cause saturation
    foreground.convertTo(foreground, -1, alpha, beta); // new_image = alpha*image + beta; used to increase contrast/saturate
    addWeighted(foreground,1.0,background,0.25,0,frameDisplay); //output into frame_with_mask

}*/


Mat copyWithPixelTransparencyUsingAlpha(Mat &background, Mat &foreground,
                                                   double start_x, double start_y,
                                                   double width, double height, int a) {
    if (/* DISABLES CODE */ (false)) { //old way that doesn't actually take into account transparency
        foreground.copyTo(background(cv::Rect(start_x, start_y, width, height)));
    } else if (start_x < background.cols && start_y < background.rows
               && (start_x + width) > 0 && (start_y + height) > 0
               && height > 0 && width > 0) {
        Mat background_original = background.clone();
        if (start_x < 0) {
            width = foreground.cols + start_x+1;
            foreground = foreground.colRange(foreground.cols-width,foreground.cols);
            start_x = 0;
        }
        if (start_y < 0) {
            height = foreground.rows + start_y+1;
            foreground = foreground.rowRange(foreground.rows-height,foreground.rows);
            start_y = 0;
        }
        if (start_x + width >= background.cols) {
            width = background.cols - start_x;
            foreground = foreground.colRange(0,width);
        }
        if (start_y + height >= background.rows) {
            height = background.rows - start_y;
            foreground = foreground.rowRange(0,height);
        }
        if (height >0 && width > 0) {
            foreground.copyTo(background(cv::Rect(start_x, start_y, width, height)));
            for (int y = 0; y < foreground.rows; ++y) {
                for (int x = 0; x < foreground.cols; ++x) {
                    cv::Vec4b &pixel_f = foreground.at<cv::Vec4b>(y, x);
                    //Below two variables are defined here instead of inside the for loop because this is empirically faster (tested on iPhone 6, Feb 6 2019).
                    cv::Vec4b &pixel_b = background.at<cv::Vec4b>(y + start_y, x + start_x);
                    cv::Vec4b &pixel_bo = background_original.at<cv::Vec4b>(y + start_y,
                                                                            x + start_x);
                    // if pixel is of color of interest
                    if (pixel_f[3] <= a) {
                        pixel_b[0] = pixel_bo[0]; //this used to be pixel_bo[1]; Typo?
                        pixel_b[1] = pixel_bo[1];
                        pixel_b[2] = pixel_bo[2];
                        pixel_b[3] = pixel_bo[3];
                        // set values to original:

                    }
                }
            }
        }
        background_original.release();
    }
    return background;
}

//Below is a significantly (up to 20x) faster algorithm to copyWithPixelTransparencyUsingAlpha that does the same thing
// Remember, x coordinates increase left to right and y coordinates increase bottom to top, ie:
// 0,0  0,1  0,2  0,3  0,4  0,5
// 1,0  1,1  1,2  1,3  1,4  1,5
// 2,0  2,1  2,2  2,3  2,4  2,5
// start_x refers to the left side of where foreground is placed, start_y refers to the top side of where foreground is placed
Mat drawImageOptimized(Mat &background, Mat &foreground, int start_x, int start_y) {
    int fg_width  = foreground.cols;
    int fg_height = foreground.rows;
    int bg_width  = background.cols;
    int bg_height = background.rows;

    if (fg_width <= 0 && fg_height <= 0 && bg_width <= 0 && bg_height <= 0) {
        cout << "Error. Invalid values for fg and bg dimensions. fg_width=" << fg_width << ", fg_height="  << fg_height << ", bg_width=" << fg_width << ", bg_height=" << bg_height << endl;
        return background;
    }

    //Check if destination area has some overlap with the background. Otherwise, nothing should be draw since there's nothing to draw.
    if (   start_x < bg_width  && start_x + fg_width  >= 0 && fg_width  > 0
        && start_y < bg_height && start_y + fg_height >= 0 && fg_height > 0) {

        //Note: col/rowRange 1st arg is inclusive, 2nd arg is exclusive
        if (start_x < 0) { //left part of fg should be cropped
            foreground = foreground.colRange(-start_x, fg_width);  //keep constant end pt length based on old fg_width before recalculations
            fg_width  += start_x; //decreases fg_width because start_x is negative
            start_x = 0;
        }
        if (start_y < 0) { //top part of fg should be cropped
            foreground = foreground.rowRange(-start_y, fg_height); //keep constant end pt length based on old fg_height before recalculations
            fg_height += start_y; //decreases fg_height because start_y is negative
            start_y = 0;
        }
        if (start_x + fg_width >= bg_width) { //right part of fg should be cropped
            fg_width  = bg_width  - start_x - 1;
            foreground = foreground.colRange(0, fg_width);  //limit length based on newly calculated fg_width
        }
        if (start_y + fg_height >= bg_height) { //bottom part of fg should be cropped
            fg_height = bg_height - start_y - 1;
            foreground = foreground.rowRange(0, fg_height); //limit length based on newly calculated fg_height
        }

        //Create an alpha mask matrix out of the fourth channel of foreground
        Mat alpha_mask( fg_height, fg_width, CV_8UC1 );
        int from_to[] = { 3,0 }; //take the 3rd (aka the alpha) channel from bg_on_fg and move it to the 0th channel of alphaPart.
        if (foreground.channels() == 4) {
            cv::mixChannels( &foreground, 1, &alpha_mask, 1, from_to, 1 ); //first 1 is # of matrices in foreground, second 1 is vice versa for background, third 1 is how many pairs are in from_to (i.e. from_to size/2)
        } else {
            alpha_mask.setTo(255);
        }

        //Threshold the mask at 127 (anything above is 255, anything below is 0)
        int threshold_value = 127;
        int if_true_value = 255;
        int threshold_type = 0; //regular old binary threshold, if greater than threshold_value, then make max_BINARY_value
        cv::threshold( alpha_mask, alpha_mask, threshold_value, if_true_value, threshold_type );

        //Copy over the foreground onto the selected parts of the background
        Mat bg_part_on_fg = background(cv::Rect(start_x, start_y, fg_width, fg_height)); //get reference to the part of the background where the foreground will be placed in

        foreground.copyTo(bg_part_on_fg, alpha_mask);
    }
    return background;
}

//NOT FINISHED YET
Mat drawImageOptimizedAlphaMixing(Mat &background, Mat &foreground, int start_x, int start_y) {
    int fg_width  = foreground.cols;
    int fg_height = foreground.rows;
    int bg_width  = background.cols;
    int bg_height = background.rows;

    //Check if destination area has some overlap with the background. Otherwise, nothing should be draw since there's nothing to draw.
    if (   start_x < bg_width  && start_x + fg_width  >= 0 && fg_width  > 0
        && start_y < bg_height && start_y + fg_height >= 0 && fg_height > 0) {

        //Note: col/rowRange 1st arg is inclusive, 2nd arg is exclusive
        if (start_x < 0) { //left part of fg should be cropped
            foreground = foreground.colRange(-start_x, fg_width);  //keep constant end pt length based on old fg_width before recalculations
            fg_width  += start_x; //decreases fg_width because start_x is negative
            start_x = 0;
        }
        if (start_y < 0) { //top part of fg should be cropped
            foreground = foreground.rowRange(-start_y, fg_height); //keep constant end pt length based on old fg_height before recalculations
            fg_height += start_y; //decreases fg_height because start_y is negative
            start_y = 0;
        }
        if (start_x + fg_width >= bg_width) { //right part of fg should be cropped
            fg_width  = bg_width  - start_x - 1;
            foreground = foreground.colRange(0, fg_width);  //limit length based on newly calculated fg_width
        }
        if (start_y + fg_height >= bg_height) { //bottom part of fg should be cropped
            fg_height = bg_height - start_y - 1;
            foreground = foreground.rowRange(0, fg_height); //limit length based on newly calculated fg_height
        }

        //Create an alpha mask matrix out of the fourth channel of foreground
        Mat alpha_mask( fg_height, fg_width, CV_8UC1 );
        int from_to[] = { 3,0 }; //take the 3rd (aka the alpha) channel from bg_on_fg and move it to the 0th channel of alphaPart.
        if (foreground.channels() == 4) {
            cv::mixChannels( &foreground, 1, &alpha_mask, 1, from_to, 1 ); //first 1 is # of matrices in foreground, second 1 is vice versa for background, third 1 is how many pairs are in from_to (i.e. from_to size/2)
        } else {
            alpha_mask.setTo(255);
        }

        //Threshold the mask at 127 (anything above is 255, anything below is 0)
        int threshold_value = 127;
        int if_true_value = 255;
        int threshold_type = 0; //regular old binary threshold, if greater than threshold_value, then make max_BINARY_value
        cv::threshold( alpha_mask, alpha_mask, threshold_value, if_true_value, threshold_type );

        //Copy over the foreground onto the selected parts of the background
        Mat bg_part_on_fg = background(cv::Rect(start_x, start_y, fg_width, fg_height)); //get reference to the part of the background where the foreground will be placed in

        //

        if (foreground.channels() == 4) {
            std::vector<cv::Mat> bgOnFgChannels;
            cv::split(bg_part_on_fg, bgOnFgChannels);

            std::vector<cv::Mat> fgChannels;
            cv::split(foreground, fgChannels);

            //addWeighted( fgChannels, alpha, src2, beta, 0.0, dst);


        } else {
            foreground.copyTo(bg_part_on_fg, alpha_mask);
        }


    }
    return background;
}


//Statistical analysis of continuous games
//incrCoeff is the amount to increase the game param for a increase in param.
// Similarly, decrCoeff is the amount to increase the game param for a decrease in param.
std::tuple<double, double, double> inferTrendFromParamHistory(double param, double incrCoeff, double decrCoeff, double stdev_lim, double currentParamVal, double breakCutoff, double startConsiderationCutoff) {

    bool exerciseDone = false;
    double increaseParamAmt = 0;

    int avg_size = 3;
    unsigned long i;
    if (paramHistory.size()<avg_size*2+1) {
        //make no change if any of the params are no exercise done
        if (!isParamNoExerciseDone(last_param) && !isParamNoExerciseDone(param)) {
            if (param<last_param) {
                increaseParamAmt = (last_param-param)*incrCoeff;
            } else {
                increaseParamAmt = -(last_param-param)*decrCoeff;
            }
        }

    } else {
        //bool isNoExerciseDoneForOneTime = false;
        double avg_old = 0;
        double avg_new = 0;
        for (i = paramHistory.size()-1; i>=paramHistory.size()-avg_size; i--) {
            if (isParamNoExerciseDone(paramHistory[i])) {
                //isNoExerciseDoneForOneTime = true;

                //make no change if any of the params are no exercise done
                goto variance_calculations;
            }
            avg_new = avg_new+paramHistory[i];
        }
        avg_new = avg_new/avg_size;
        for (; i>=paramHistory.size()-avg_size*2; i--) {
            if (isParamNoExerciseDone(paramHistory[i])) {
                //isNoExerciseDoneForOneTime = true;

                //make no change if any of the params are no exercise done
                goto variance_calculations;
            }
            avg_old = avg_old+paramHistory[i];
        }
        avg_old = avg_old/avg_size;
        //make no change if any of the params are no exercise done
        if (avg_new<avg_old) {
            increaseParamAmt = (avg_old-avg_new)*incrCoeff; //increase balloon
        } else {
            increaseParamAmt = -(avg_old-avg_new)*decrCoeff; //decrease
        }


        /*int extra_avg_size = 5;
        if (paramHistory.size()>=avg_size*2+extra_avg_size+1 && currentBalloonSize > BALLOON_STARTING_SIZE*3) {
        double avg_veryold = 0;
        for (; i>=paramHistory.size()-(avg_size*2+extra_avg_size); i--) {
        avg_veryold = avg_veryold+paramHistory[i];
        }
        double avg_medium = 0.5*avg_new + 0.5*avg_old;
        avg_veryold = avg_veryold/extra_avg_size;
        cout << " " << std::to_string(avg_medium-avg_veryold) << " ";
        if ( abs(avg_medium-avg_veryold) < 0.01) {
        currentBalloonSize = 10000000;
        }
        }*/

        /*double avg_medium = 0.5*avg_new + 0.5*avg_old;
        double variance;
        i = paramHistory.size()-1;
        for (; i>=paramHistory.size()-avg_size*2; i--) {
        variance = variance+(paramHistory[i]-avg_medium)*(paramHistory[i]-avg_medium);
        }
        variance = variance/(avg_size*2-1); //sample variance equation, i.e. bessel's correction
        double stdev_lim = 0.01;
        cout << " myvariance: " << std::to_string(variance) << " ";
        //__android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", " myvariance: " + std::to_string(variance) + " ");
        if ( variance < stdev_lim*stdev_lim && currentBalloonSize > BALLOON_STARTING_SIZE*3) {
        currentBalloonSize = 10000000;
        }*/
        variance_calculations:
        //int var_size = 45; //make it 20 on Rahul's Android or MoTrack Android when on high resolution and no frame delay
        int var_size = 0;
        int no_hand_ct = 0;
        const int min_var_size = 4; //try to get at least this many frames to calculate variance; otherwise, don't use it
        const int ideal_var_size_ms = 1500; //in ms, not frames; ideally, how far back are you checking the variance
        chrono::steady_clock::time_point timeAtVarianceCalc = chrono::steady_clock::now();
        if (paramHistory.size() >= min_var_size) { //only need to calculate variance if there has been at least some frames
            double sum_p  = 0;
            double sum_p2 = 0;
            i = paramHistory.size()-1;
            //for (; i>=paramHistory.size()-var_size; i--) {
            do {
                var_size++;

                double this_param = paramHistory[i];
                if (isParamNoHand(this_param)) {
                    //isNoExerciseDoneForOneTime = true;
                    //goto endcalculations;
                    no_hand_ct++;

                } else if (isParamNoExerciseDone(this_param)) {
                    goto endcalculations;

                }  else {
                    sum_p  = sum_p  + this_param;
                    sum_p2 = sum_p2 + this_param*this_param;
                }

                //need to do i==0, not just check after i<0 in a for loop because since i is unsigned, doing i-- when i=0 makes i still positve as the largest possible value
                if (i==0) break;
                else i--;

                //Use <, not <= for min_var_size, bc one more iteration will be run if true
            } while (var_size < min_var_size || std::chrono::duration_cast<std::chrono::milliseconds>(timeAtVarianceCalc - paramHistoryTimes[i]).count()<=ideal_var_size_ms);

            if (currentParamVal+increaseParamAmt > startConsiderationCutoff) {
                if (var_size <= min_var_size) {
                    exerciseDone = false;
                } else if (var_size == no_hand_ct) {
                    exerciseDone = true;
                } else {
                    //double variance = ( (var_size-no_hand_ct)*sum_p2 - sum_p*sum_p)/( (var_size-no_hand_ct)*(var_size-no_hand_ct-1) ); //sample variance shortcut formula
                    double variance = ( sum_p2 - sum_p*sum_p/(var_size-no_hand_ct) )/(var_size-no_hand_ct-1); //sample variance shortcut formula
                    if ( no_hand_ct == 0 && variance < stdev_lim*stdev_lim) {
                        exerciseDone = true;
                    }
                }
            }

        }

    }

    endcalculations:
    //cout << currentParamVal << " " << param << " " << (currentParamVal+increaseParamAmt) << endl;
    currentParamVal += increaseParamAmt;
    if (currentParamVal > breakCutoff) { //popped
        //balloon pops
        currentParamVal = -1;
        exerciseDone = true;
    }

    return std::make_tuple(currentParamVal, exerciseDone ? 1.0 : -1.0, increaseParamAmt);
}

// TODO: move this //on Mar 19, 2020, this exact function was in 3 places, so removed the other too
// adds in hand to give the user an idea of how big their hand should be
void addHandForSizeReference(Mat& frameDisplay, bool isPortrait, int speedReq, bool isTutorial) {
    if (sizeRefImgOrig.data != NULL) {
        int blursize = 39; //*1.0/(1280*720)*frameDisplay.rows*frameDisplay.cols;
        if (fgLocAlg >= 3000 && fgLocAlg < 5000) blursize = 0;
        if (speedReq == 1) blursize /= 2;

        if (speedReq == 3 || blursize == 0) {
            //Don't do anything here to be as fast as possible for  speedReq == 3

        } else if (speedReq == 2) {
            int origRows = frameDisplay.rows;
            int origCols = frameDisplay.cols;
            double scale = 0.2;
            Mat frameDisplay2; //for some reason, it wasn't working when I tried to just use the same matrix instead of an intermediate variable frameDisplay2
            cv::resize(frameDisplay, frameDisplay2, cv::Size(), scale, scale);
            cv::blur(frameDisplay2, frameDisplay2, Size((int)(blursize*scale), (int)(blursize*scale)));
            if (blursize*scale > 0) cv::resize(frameDisplay2, frameDisplay, cv::Size(origCols, origRows));

        } else {
            if (blursize > 0) cv::blur(frameDisplay, frameDisplay, Size(blursize, blursize));
        }

        // check if needs resizing
        //if ((int(frameDisplay.rows-1) == sizeRefImg.rows) || int(frameDisplay.cols-1) == sizeRefImg.cols) {
        vector<tuple<string,cv::Scalar>> toWrite;
        if (isPortrait) {
            cv::resize(sizeRefImgOrig, sizeRefImg,
                       cv::Size(), frameDisplay.rows/double(sizeRefImgOrig.rows)/2.0*4/3, frameDisplay.rows/double(sizeRefImgOrig.rows)/2.0*4/3,
                       INTER_NEAREST);
            int top = frameDisplay.rows - sizeRefImg.rows;
            int bottom = 0;
            int left = (frameDisplay.cols - sizeRefImg.cols)/2;
            int right = frameDisplay.cols - sizeRefImg.cols - left;
            cv::copyMakeBorder(sizeRefImg, sizeRefImg, top, bottom, left, right, BORDER_CONSTANT, Scalar(0,0,0,0) );
            if (sizeRefImg.channels() >= 4) { //MUST DO THIS EVERY TIME sizeRefImg is resized
                vector<Mat> channels;
                split(sizeRefImg,channels);
                sizeRefImgAlpha = channels[3];
            }

            /*toWrite =  {
                make_tuple("Your hand should be",       cv::Scalar(255,0,0,255)),
                make_tuple("roughly the same size",     cv::Scalar(255,0,0,255)),
                make_tuple("as the one on the screen",  cv::Scalar(255,0,0,255))
            };*/
            if (useGreenTint > 0 && (isTutorial || fgLocAlg == 4 || fgLocAlg == 6 || fgLocAlg == 8 || fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694)) {
                toWrite =  {
                        make_tuple("Keep your hand up",     cv::Scalar(255,0,0,255)),
                        make_tuple("steady. Great!",        cv::Scalar(255,0,0,255))
                };
            } else {
                toWrite =  {
                        make_tuple("Line your hand up",     cv::Scalar(255,0,0,255)),
                        make_tuple("with the one on",       cv::Scalar(255,0,0,255)),
                        make_tuple("the screen",            cv::Scalar(255,0,0,255))
                };
            }

        } else {
            /*cv::resize(sizeRefImg, sizeRefImg,
                       cv::Size(int(frameDisplay.rows - 1), int(frameDisplay.rows - 1)), 0, 0,
                       INTER_NEAREST);*/
            cv::resize(sizeRefImgOrig, sizeRefImg,
                       cv::Size(), frameDisplay.rows/double(sizeRefImgOrig.cols)/2.0, frameDisplay.rows/double(sizeRefImgOrig.cols)/2.0,
                       INTER_NEAREST);
            int top = frameDisplay.rows - sizeRefImg.rows;
            int bottom = 0;
            int left = (frameDisplay.cols - sizeRefImg.cols)/2;
            int right = frameDisplay.cols - sizeRefImg.cols - left;
            cv::copyMakeBorder(sizeRefImg, sizeRefImg, top, bottom, left, right, BORDER_CONSTANT, Scalar(0,0,0,0) );
            if (sizeRefImg.channels() >= 4) { //MUST DO THIS EVERY TIME sizeRefImg is resized
                vector<Mat> channels;
                split(sizeRefImg,channels);
                sizeRefImgAlpha = channels[3];
            }
            /*
            toWrite =  {
                make_tuple("Your hand",     cv::Scalar(255,0,0,255)),
                make_tuple("should be",     cv::Scalar(255,0,0,255)),
                make_tuple("roughly the",   cv::Scalar(255,0,0,255)),
                make_tuple("same size",     cv::Scalar(255,0,0,255)),
                make_tuple("as the one",    cv::Scalar(255,0,0,255)),
                make_tuple("on the screen", cv::Scalar(255,0,0,255))
            };*/
            /*toWrite =  {
                make_tuple("Your hand",     cv::Scalar(255,0,0,255)),
                make_tuple("should be",     cv::Scalar(255,0,0,255)),
                make_tuple("roughly",       cv::Scalar(255,0,0,255)),
                make_tuple("the same",      cv::Scalar(255,0,0,255)),
                make_tuple("size as",       cv::Scalar(255,0,0,255)),
                make_tuple("the one on",    cv::Scalar(255,0,0,255)),
                make_tuple("the screen",    cv::Scalar(255,0,0,255))
            };*/
            if (useGreenTint > 0 && (fgLocAlg == 4 || fgLocAlg == 6 || fgLocAlg == 8 || fgLocAlg == 1680 || fgLocAlg == 1684 || fgLocAlg == 1690 || fgLocAlg == 1694)) {
                toWrite =  {
                        make_tuple("Keep your",         cv::Scalar(255,0,0,255)),
                        make_tuple("hand steady!",       cv::Scalar(255,0,0,255))
                };
            } else {
                toWrite =  {
                        make_tuple("Line your",     cv::Scalar(255,0,0,255)),
                        make_tuple("hand up",       cv::Scalar(255,0,0,255)),
                        make_tuple("with the",      cv::Scalar(255,0,0,255)),
                        make_tuple("one on the",    cv::Scalar(255,0,0,255)),
                        make_tuple("screen",        cv::Scalar(255,0,0,255))
                };
            }

        }
        //}

        /*string s1 = ;
        string s2 = ;
        double fontScale = writeText(frameDisplay, s1, -1, int(frameDisplay.rows * 0.20), -1.0);
        writeText(frameDisplay, s2, -1, int(frameDisplay.rows * 0.45));*/

        if (shouldWriteOnIm) {
            cv::rectangle(frameDisplay, textWriteRectangle, textWriteRectangleBackground, -1, 8);
        }

        if (exercise_game>0 || motionsMap.numSkipsTot%2==0) {
            //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION HERE, YOU MUST HAND CENTER DETECTION DURING REDBOX AS IMPLEMENTED IN ALGORITHM 4 of CALIBRATION.CPP
            Mat frameDisplayCopiedSizeReference = frameDisplay.clone();
            if (isPortrait) {
                //cv::resize(fgmask, fgmask, frameDisplay.size(),0,0, INTER_NEAREST );
                double scale = 0.3*(frameDisplay.cols*frameDisplay.rows)/(sizeRefImgOrig.rows*sizeRefImgOrig.cols);
                //cv::resize(sizeRefImgOrig, sizeRefImg, Size(), 0.05, 0.05 );
                //drawImageOptimized(frameDisplay, sizeRefImg, frameDisplay.cols/2-sizeRefImg.cols/2, frameDisplay.rows-sizeRefImg.rows-1);
                drawImageOptimized(frameDisplayCopiedSizeReference, sizeRefImg, 0, 0);
            } else {
                drawImageOptimized(frameDisplayCopiedSizeReference, sizeRefImg, 0, 0);
            }
            double handSizeReferenceTransparency = 0.8; //on [0,1] interval. Closer to 1 -> hand size reference is more visible
            addWeighted( frameDisplayCopiedSizeReference, handSizeReferenceTransparency, frameDisplay, 1-handSizeReferenceTransparency, 0.0, frameDisplay);
        }


        if (!badBackground) {
            writeText_onRect(frameDisplay, toWrite);
        } else {
            writeBadBackgroundText(frameDisplay);
        }
    }

#if MY_OS==ANDROID_OS
    //Must be done after the blur
    //This is the line that separates the text at the top/side from the rest of the screen
    if (isPortrait) {
        cv::line(frameDisplay,cv::Point(0,textWriteRectangle.height),cv::Point(frameDisplay.cols,textWriteRectangle.height),cv::Scalar(0,0,0,255),5);
    } else {
        cv::line(frameDisplay,cv::Point(textWriteRectangle.width,0),cv::Point(textWriteRectangle.width,frameDisplay.rows),cv::Scalar(0,0,0,255),5);
    }
#endif
}

//string editing methods for writing on screen
vector<tuple<string,cv::Scalar>> splitTextUpByWord(vector<tuple<string,cv::Scalar>> toWrite, string inputText, Scalar color) {
    std::size_t loc_start = 0;
    while (loc_start != std::string::npos) { //npos means no matches were found
        std::size_t loc_end = inputText.find(' ',loc_start); //will return index of space or npos if no spaces are found
        std::string phrase = inputText.substr(loc_start, loc_end-loc_start); //if loc_end is npos, it will go until the end of string
        if (loc_end != std::string::npos) {
            loc_start = loc_end+1;
        } else {
            loc_start = loc_end;
        }
        if (phrase.length() > 0) { //ignore extra whitespace
            toWrite.push_back(make_tuple(phrase, color));
        }

    }
    return toWrite;
}


vector<tuple<string,cv::Scalar>> splitTextUpBySemicolon(vector<tuple<string,cv::Scalar>> toWrite, string inputText, Scalar color) {
    std::size_t loc_start = 0;
    while (loc_start != std::string::npos) { //npos means no matches were found
        std::size_t loc_end = inputText.find(';',loc_start); //will return index of space or npos if no spaces are found
        std::string phrase = inputText.substr(loc_start, loc_end-loc_start); //if loc_end is npos, it will go until the end of string
        if (loc_end != std::string::npos) {
            loc_start = loc_end+1;
        } else {
            loc_start = loc_end;
        }
        if (phrase.length() > 0) { //ignore extra whitespace
            toWrite.push_back(make_tuple(phrase, color));
        }

    }
    return toWrite;
}

    bool isParamNoHand(double param) {
        //check if value is close enough to PARAM_NO_HAND, while realizing that both are doubles and comparing doubles is tricky
        bool no_hand = abs(param - PARAM_NO_HAND)/(1.0+abs(PARAM_NO_HAND)) < 0.00000001;
        return no_hand;
    }

    bool isParamNotValid(double param) {
        //check if value is close enough to PARAM_NOT_VALID, while realizing that both are doubles and comparing doubles is tricky
        bool not_valid = abs(param - PARAM_NOT_VALID)/(1.0+abs(PARAM_NOT_VALID)) < 0.00000001;
        return not_valid;
    }

    bool isParamNoExerciseDone(double param) {
        bool not_valid = isParamNotValid(param);
        bool no_hand   =   isParamNoHand(param);
        return no_hand || not_valid;
    }

    std::string paramToStr(double param) {
        if (isParamNoHand(param)) return "No Hand";
        else if (isParamNotValid(param)) return "Not Valid";
        else return std::to_string(param);
    }

    double subtractParams(double paramEst, double paramLabel) {
        bool isParamEstNoExerciseDone   = isParamNoExerciseDone(paramEst);
        bool isParamLabelNoExerciseDone = isParamNoExerciseDone(paramLabel);

        if ( isParamEstNoExerciseDone && isParamLabelNoExerciseDone ) {
            if ( abs(paramEst-paramLabel) < 1.0/abs(paramEst+paramLabel) ) { //check if paramEst is the same as paramLabel
                return 0;
            } else {
                return nan(""); //return nan, http://www.cplusplus.com/reference/cmath/nan-function/
            }

        } else if ( isParamEstNoExerciseDone || isParamLabelNoExerciseDone ) { //if only one is no exercise done, then return nan
            return nan(""); //return nan, http://www.cplusplus.com/reference/cmath/nan-function/

        } else return paramEst-paramLabel;
    }


#ifdef EXTERN_C
}
#endif
