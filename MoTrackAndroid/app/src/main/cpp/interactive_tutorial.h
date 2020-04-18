//
// Created by benka on 1/27/20.
//

#ifndef MOTRACKANDROID_INTERACTIVE_TUTORIAL_H
#define MOTRACKANDROID_INTERACTIVE_TUTORIAL_H

#include <opencv2/opencv.hpp>
#include "calibration.h"

#ifdef EXTERN_C
extern "C" {
#endif

// INTERACTIVE TUTORIAL
double calculate_fist_param(cv::Mat& fgmask, double radius, distInfo d);
bool teachOpenPalm(cv::Mat& frameDisplay, cv::Mat& fgmask, double radius, distInfo d, double ratio);
bool teachClosedFist(cv::Mat& frameDisplay, cv::Mat& fgmask, double radius, distInfo d, double ratio);
bool teachSpecificExercise(cv::Mat& frameDisplay,
                           bool isPortrait, cv::Mat &fgmask, double ratio, int);
bool doGameIntro(cv::Mat& frameDisplay,
                 bool isPortrait, cv::Mat &fgmask, double ratio);

void setInteractiveTutorialExercise();
bool checkIfLinedUp(cv::Mat &fgmask, double ratio);
bool runInteractiveTutorial(cv::Mat& frameDisplay, bool isPortrait, cv::Mat &fgmask, double ratio);
bool generalIntro(cv::Mat& frameDisplay, bool isPortrait, cv::Mat &fgmask, double ratio);
bool lineUpHand(cv::Mat& frameDisplay, bool isPortrait, cv::Mat &fgmask, double ratio);
// END INTERACTIVE TUTORIAL

#ifdef EXTERN_C
}
#endif
#endif //MOTRACKANDROID_INTERACTIVE_TUTORIAL_H
