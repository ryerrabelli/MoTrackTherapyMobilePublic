//
//  hook_fist_caterpillar.hpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 5/23/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_HOOK_FIST_CATERPILLAR_H
#define MOTRACKTHERAPYMOBILE_HOOK_FIST_CATERPILLAR_H

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
    Java_com_motracktherapy_motrack_CVActivity_setCaterpillarGameScenery(JNIEnv *env, jobject, long addr);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setCaterpillarGameScenery(cv::Mat picture);
    
    
#endif
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_caterpillar_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    
    
#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_HOOK_FIST_CATERPILLAR_H */
