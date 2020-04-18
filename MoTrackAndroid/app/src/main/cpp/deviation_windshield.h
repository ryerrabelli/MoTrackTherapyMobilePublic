//
// Created by Rahul Yerrabelli on 11/30/18.
//

#ifndef MOTRACKTHERAPYMOBILE_ULNAR_RADIAL_WINDSHIELD_H
#define MOTRACKTHERAPYMOBILE_ULNAR_RADIAL_WINDSHIELD_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif

#if MY_OS==ANDROID_OS
extern "C" {

void
Java_com_motracktherapy_motrack_CVActivity_setCarFrame(JNIEnv *env, jobject, long addrCarFrame, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldGameScenery(JNIEnv *env, jobject, long addrScenery, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldWiperBottom(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldWiperTop(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

void
Java_com_motracktherapy_motrack_CVActivity_setWindshieldGameSteeringWheel(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
void setCarFrame(cv::Mat picture);
void setWindshieldGameScenery(cv::Mat picture);
void setWindshieldWiperBottom(cv::Mat picture);
void setWindshieldWiperTop(cv::Mat picture);

#endif


int do_windshield_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double);


#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_ULNAR_RADIAL_WINDSHIELD_H
