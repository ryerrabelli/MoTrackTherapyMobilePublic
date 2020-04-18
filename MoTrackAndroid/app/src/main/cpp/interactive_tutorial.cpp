//
// Created by benka on 1/27/20.
//

#include "interactive_tutorial.h"
#include "calibration.h"
//#include "native-lib.h"
#include "audio_player.h"
#include "fist_analysis.h"
////#include "all_access_vars.h"

using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif
/*
 * Interactive tutorial
 */

bool learnedMotion = false;

//&
/*
bool teachOpenPalm(Mat& frameDisplay, Mat& fgmask, double radius, distInfo d, double ratio) {

    double param = calculate_fist_param(fgmask, radius, d);
    // give written instruction
    vector<tuple<string,cv::Scalar>> toWrite;

    if (param == PARAM_NO_HAND || param == PARAM_NOT_VALID) {
        toWrite =  {
                make_tuple("No hand detected",     cv::Scalar(255,0,0,255))
        };
    } else if (param >= 0.2 ) { // doing motion successfully
        auto time_diff =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - startTime;
        if (time_diff > holdExerciseLength) { //motion completed
            // add green tint to image
            useGreenTint = 10;
            playAudioFileCorrect();
            return true;
        } else {
            toWrite = {
                    make_tuple("Hold your position", cv::Scalar(0, 255, 0, 255)),
                    //make_tuple("your palm faces",       cv::Scalar(255,0,0,255)),
                    //make_tuple("the screen",            cv::Scalar(255,0,0,255))
            };
        }
    } else {
        toWrite =  {
                make_tuple("Open your fist more ",     cv::Scalar(255,0,0,255)),
                //make_tuple("your palm faces",       cv::Scalar(255,0,0,255)),
                //make_tuple("the screen",            cv::Scalar(255,0,0,255))
        };
    }

    writeText_onRect(frameDisplay, toWrite);
    // draw outward arrows
    //draw_fist_arrows(frameDisplay, 0,  radius, d, ratio);
    return false;
}

bool teachClosedFist(Mat& frameDisplay, Mat& fgmask, double radius, distInfo d, double ratio) {
    double param = calculate_fist_param(fgmask, radius, d);
    // give written instruction
    vector<tuple<string,cv::Scalar>> toWrite;

    if (param == PARAM_NO_HAND || param == PARAM_NOT_VALID) {
        toWrite =  {
                make_tuple("No hand detected",     cv::Scalar(255,0,0,255))
        };
    } else if (param <= -0.15 ) { // doing motion successfully
        auto time_diff =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - startTime;
        if (time_diff > holdExerciseLength) { //motion completed
            // add green tint to image
            useGreenTint = 10;
            playAudioFileCorrect();
            return true;
        } else {
            toWrite = {
                    make_tuple("Hold your position", cv::Scalar(0, 255, 0, 255)),
                    //make_tuple("your palm faces",       cv::Scalar(255,0,0,255)),
                    //make_tuple("the screen",            cv::Scalar(255,0,0,255))
            };
        }
    } else {
        toWrite =  {
                make_tuple("Close your fist more",     cv::Scalar(255,0,0,255)),
                //make_tuple("your palm faces",       cv::Scalar(255,0,0,255)),
                //make_tuple("the screen",            cv::Scalar(255,0,0,255))
        };
    }
    writeText_onRect(frameDisplay, toWrite);
    draw_fist_arrows(frameDisplay, 1,  radius, d, ratio);
    return false;
}
 */

// Will move on to the next exercise tutorial.
// If done with all exercises, return true. Else return false
bool done_with_exercise() {
    // incremenet exercise index
    interactivetutorial_exercise_index++;
    int size = static_cast<int>(interactivetutorial_exercises.size());
    if (interactivetutorial_exercise_index >= size) {
        return true;
    }
    return false;
}

// Returns true when finished all exercise tutorials
bool teachSpecificExercise(Mat& frameDisplay,
                           bool isPortrait, Mat &fgmask, double ratio, int exerciseNumToDo) {
    // to heavily dim background
    //showbackground = 3;
    //removeBackground(frameDisplay,fgmask,frameDisplay);
    bool doneWithExercise = false;
    double param;
    distInfo d = get_distance_transform(fgmask);
    double radius = d.dist.at<float>(d.maxLoc.y,d.maxLoc.x);
    if (exerciseNumToDo == INTERACTIVETUTORIAL_OPENFIST) {
        cv::Point dmaxLoc;
        double radius;
        int highest_y;
        std::tie(param, dmaxLoc, radius, highest_y) = analyze_fist(fgmask, frameDisplay, 0, ratio, true);
        //&doneWithExercise = teachOpenPalm(frameDisplay, fgmask, radius, d, ratio);
    } else if (exerciseNumToDo == INTERACTIVETUTORIAL_CLOSEFIST) {
        //&doneWithExercise =  teachClosedFist(frameDisplay, fgmask, radius, d, ratio);
    }

    // give written instruction
    vector<tuple<string,cv::Scalar>> toWrite;
    if (param == PARAM_NO_HAND || param == PARAM_NOT_VALID) {
        toWrite =  {
                make_tuple("No hand detected",     cv::Scalar(255,0,0,255))
        };
    } else if (param <= -0.15 ) { // doing motion successfully
        auto time_diff =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - startTime;
        if (time_diff > holdExerciseLength) { //motion completed
            // add green tint to image
            useGreenTint = 10;
            playAudioFileCorrect();
            return true;
        } else {
            toWrite = {
                    make_tuple("Hold your position", cv::Scalar(0, 255, 0, 255)),
                    //make_tuple("your palm faces",       cv::Scalar(255,0,0,255)),
                    //make_tuple("the screen",            cv::Scalar(255,0,0,255))
            };
        }
    } else {
        toWrite =  {
                make_tuple("Close your fist more",     cv::Scalar(255,0,0,255)),
                //make_tuple("your palm faces",       cv::Scalar(255,0,0,255)),
                //make_tuple("the screen",            cv::Scalar(255,0,0,255))
        };
    }
    writeText_onRect(frameDisplay, toWrite);
    // check if completed exercise
    if (doneWithExercise) {
        // increment the exercise to move on to the next one
        // return true if finished all exercises, else return false
        return done_with_exercise();
    }
    return false;
}

bool doGameIntro(Mat& frameDisplay,
                 bool isPortrait, Mat &fgmask, double ratio) {
    return teachSpecificExercise(frameDisplay,isPortrait, fgmask,
            ratio, interactivetutorial_exercises[interactivetutorial_exercise_index]);
    //teachSpecificGame();
}

// Picks the Interactive Tutorial Exercise to set for the given game
void setInteractiveTutorialExercise() {
    interactivetutorial_exercise_index = 0;
    interactivetutorial_exercises = mapOfInteractiveExercises.at(exercise_game);
}

bool runInteractiveTutorial(Mat& frameDisplay, bool isPortrait, Mat &fgmask, double ratio) {
    if (interactivetutorial_stage == INTERACTIVETUTORIAL_GENERAL_INTRO) {
        bool doneGeneralIntro = generalIntro(frameDisplay,  isPortrait, fgmask, ratio);
        if (doneGeneralIntro) {
            interactivetutorial_stage = INTERACTIVETUTORIAL_EXERCISE_INTRO;
        }
    }
    else if (interactivetutorial_stage == INTERACTIVETUTORIAL_EXERCISE_INTRO) {
        bool doneGameIntro = doGameIntro(frameDisplay,isPortrait, fgmask, ratio);
        if (doneGameIntro) {
            return true;
        }
    }
    return false;
}

bool generalIntro(Mat& frameDisplay, bool isPortrait, Mat &fgmask, double ratio) {
    // 1) Place your phone down on a flat surface, with the screen facing up.
    // 2) Make sure there are no lights directly overhead
    //      (this makes it difficult for the camera to focus on your hand).
    // 3) Make sure your sound is on. The different sounds
    //      will let you know when different things are happening in the games.
    // 4) Line up your hand with the example hand on the screen.
    //      make sure your hand is the correct distance from the camera by matching the example size
    // 5) when your hand is lined up correctly for
    //      long enough, the screen will flash green and progress on
    bool doneLiningUpHand = lineUpHand(frameDisplay, isPortrait, fgmask, ratio);
    // 6) Begin game!
    if (doneLiningUpHand) {
        return true;
    }
    return false;
}


// returns true when has had hand lined up for two seconds
bool lineUpHand(Mat& frameDisplay, bool isPortrait, Mat &fgmask, double ratio) {
    // 4) Line up your hand with the example hand on the screen.
    //      make sure your hand is the correct distance from the camera by matching the example size
    // 5) when your hand is lined up correctly for
    //      long enough, the screen will flash green and progress on

    // add hand example
    addHandForSizeReference(frameDisplay, isPortrait, 2, true);

    // check if lined up
    if (interactiveTutorialStartTime < 0) {
        interactiveTutorialStartTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    if (checkIfLinedUp(fgmask, ratio)) {
        useGreenTint = 2;
        auto time_diff =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - interactiveTutorialStartTime;
        if (time_diff > 2.0) {
            return true;
        }
    } else {
        interactiveTutorialStartTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    return false;
}

bool checkIfLinedUp(Mat &fgmask, double ratio) {
    if (sizeRefImgAlpha.data != NULL) {

        int nonzeroSizeRef = cv::countNonZero(sizeRefImgAlpha) *ratio*ratio; //multiply by ratio twice because area not length

        //Check the center of fgmask to see if it is in the redbox area
        //NOTE: IF YOU CHANGE THE REDBOX IMAGE LOCATION (in native-lib), YOU MUST CHANGE THIS AS WELL
        Moments m = moments(fgmask,true);
        Point p(m.m10/m.m00, m.m01/m.m00);

        int nonzero2 = cv::countNonZero(fgmask);
        /*
        bool isCenterXCorrect = p.x > (fgmask.cols-sizeRefImg.cols*ratio)/2 && p.x < (fgmask.cols+sizeRefImg.cols*ratio)/2;
        bool isCenterYCorrect = p.y > fgmask.rows-sizeRefImg.rows*ratio;
        bool isCenterCorrect = isCenterXCorrect && isCenterYCorrect;*/
        bool isCenterCorrect = (sizeRefImgAlpha.data != NULL && nonzero2>0) ? sizeRefImgAlpha.at<uchar>(p.y/ratio,p.x/ratio) > 0 : false;

        //On Sept 8th, 2019, I (Rahul) added the abs(.) because I think it was a bug that had somehow gone unnoticed for a while... ?. I added it each time this type of test came up (total of 4 matches, including the 4th one I added that day for alg 3000).
        if ( abs(nonzero2-nonzeroSizeRef)*1.0/nonzeroSizeRef > 0.2 || !isCenterCorrect) { //checks if hand is significantly bigger than reference hand
            return false;
        }
        return true;
    } else {
        return false;
    }
}
/*
 * End of interactive tutorial (also calls addhandforsizeference below)
 */
#ifdef EXTERN_C
}
#endif
