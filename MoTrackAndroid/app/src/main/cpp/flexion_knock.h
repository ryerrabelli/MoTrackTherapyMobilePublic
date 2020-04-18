//
// Created by Rahul Yerrabelli on 12/18/18.
//

#ifndef MOTRACKTHERAPYMOBILE_FLEXION_KNOCK_H
#define MOTRACKTHERAPYMOBILE_FLEXION_KNOCK_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


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
Java_com_motracktherapy_motrack_CVActivity_setKnockGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * -------------------------------------------------------------------------------------------------
 *                                     iOS INTERFACE FUNCTIONS
 * -------------------------------------------------------------------------------------------------
 */

void
setKnockGameScenery(cv::Mat knockImageMat);

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */


int do_knock_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int, double);

#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_FLEXION_KNOCK_H
