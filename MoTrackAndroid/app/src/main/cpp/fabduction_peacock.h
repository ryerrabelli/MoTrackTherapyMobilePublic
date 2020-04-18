//
//  fabduction_peacock.hpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/21/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_FABDUCTION_PEACOCK_H
#define MOTRACKTHERAPYMOBILE_FABDUCTION_PEACOCK_H

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
    Java_com_motracktherapy_motrack_CVActivity_setPeacockGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setPeacockGamePeacockBody(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setPeacockGameFeather(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setPeacockGameScenery(cv::Mat picture);
    void setPeacockGamePeacockBody(cv::Mat picture);
    void setPeacockGameFeather(cv::Mat picture);
    
#endif
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_peacock_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    
    
#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_FABDUCTION_PEACOCK_H */
