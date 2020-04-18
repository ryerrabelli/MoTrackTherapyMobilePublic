//
//  pronation_dial.h
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/19/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_PRONATION_DIAL_H
#define MOTRACKTHERAPYMOBILE_PRONATION_DIAL_H

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
    Java_com_motracktherapy_motrack_CVActivity_setDialFrame(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

    void
    Java_com_motracktherapy_motrack_CVActivity_setOvenBackground(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

    void
    Java_com_motracktherapy_motrack_CVActivity_setDialPointer(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setDialFrame(cv::Mat picture);
    void setOvenBackground(cv::Mat picture);
    void setDialPointer(cv::Mat picture);

    #endif

    double do_dial_game(cv::Mat fgmask, cv::Mat &frame_with_mask, double ratio);


#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_PRONATION_DIAL_H */
