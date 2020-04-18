//
//  topposition_alligator.hpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/30/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_TOPPOSITION_ALLIGATOR_H
#define MOTRACKTHERAPYMOBILE_TOPPOSITION_ALLIGATOR_H

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
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameScenery(JNIEnv *env, jobject, long addr);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameAlligatorBody(JNIEnv *env, jobject, long addr);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameAlligatorUpperMouth(JNIEnv *env, jobject, long addr);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setAlligatorGameAlligatorLowerMouth(JNIEnv *env, jobject, long addr);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setAlligatorGameScenery(cv::Mat picture);
    
    void setAlligatorGameAlligatorBody(cv::Mat picture);
    
    void setAlligatorUpperMouth(cv::Mat picture);
    void setAlligatorLowerMouth(cv::Mat picture);
    
    
#endif
        
    // main method that does game graphics and rotation calculation, returning angle
    double do_alligator_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    
    
#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_TOPPOSITION_ALLIGATOR_H */
