//
// Created by benka on 11/25/18.
//

#ifndef MOTRACKTHERAPYMOBILE_FIST_H
#define MOTRACKTHERAPYMOBILE_FIST_H

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

#include <opencv2/opencv.hpp>


#ifdef EXTERN_C
extern "C" {
#endif

// Squeeze bottle game
extern double percentToFill;
extern bool updateCup;
extern cv::Mat bottleGameScenery;
extern cv::Mat lemonBottle;
extern cv::Mat cup;
extern cv::Mat cupResized;
// Pump balloon game
extern const double BALLOON_STARTING_SIZE;
extern double currentBalloonSize;
extern cv::Mat balloonGameScenery;
extern cv::Mat balloon;
extern cv::Mat heliumTank;
extern cv::Mat poppedBalloon;

#if (MY_OS==MAC_OS || MY_OS==LINUX_OS)
    extern double lastFistRadius;
    extern cv::Point lastFistMaxLoc;
#endif
    

#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
*/
extern "C" {

void
Java_com_motracktherapy_motrack_CVActivity_setBottleGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
void
Java_com_motracktherapy_motrack_CVActivity_setLemonBottle(JNIEnv *env, jobject, long addrLemonImage,
                                                          jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setCupImage(JNIEnv *env, jobject, long addrCupImage,
                                                       jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setBalloonGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setBalloon(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setHeliumTank(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setPoppedBalloon(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
    
void setBottleGameScenery(cv::Mat picture);
void setCupImage(cv::Mat picture);
void setLemonBottle(cv::Mat picture);

void setBalloonGameScenery(cv::Mat picture);
void setBalloon(cv::Mat picture);
void setHeliumTank(cv::Mat picture);
void setPoppedBalloon(cv::Mat picture);

void setCurveArrows(cv::Mat left, cv::Mat right);

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */
    
//double normalizeOneAndNegativeOne(double p, double min_p, double max_p);
void draw_fist_arrows(cv::Mat &frame_with_mask, int dir, double radius, distInfo d, double ratio);
std::tuple<double, cv::Point, double, int> analyze_fist(cv::Mat fgmask, cv::Mat &frame_with_mask, int dir, double ratio, bool include_arrows);

double do_plain_fist(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);

double do_squeeze_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);


double get_max_balloon_param();

double get_min_considering_balloon_param();

double calculate_score_increase_for_balloon_game();

double do_balloon_game(cv::Mat fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    



#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_FIST_H
