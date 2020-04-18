//
//  pronation_ketchup.hpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/11/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_PRONATION_KETCHUP_H
#define MOTRACKTHERAPYMOBILE_PRONATION_KETCHUP_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif
    
    extern const double KETCHUP_STARTING_PARAM;
    extern double currentKetchupParam;
    extern double ketchupParamIncreaseAmt;

#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameScenery(JNIEnv *env, jobject, long addr);

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGamePlateWithFries1(JNIEnv *env, jobject, long addr);

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGamePlateWithFries2(JNIEnv *env, jobject, long addr);

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameKetchupBottle(JNIEnv *env, jobject, long addr);

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameKetchupBottleCap(JNIEnv *env, jobject, long addr);

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameSingleFryWithoutKetchup(JNIEnv *env, jobject, long addr);

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameSingleFryWithKetchup(JNIEnv *env, jobject, long addr);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    
    void setKetchupGameScenery(cv::Mat picture);
    void setKetchupGamePlateWithFries1(cv::Mat picture);
    void setKetchupGamePlateWithFries2(cv::Mat picture);
    void setKetchupGameKetchupBottle(cv::Mat picture);
    void setKetchupGameKetchupBottleCap(cv::Mat picture);
    
    void setKetchupGameSingleFryWithoutKetchup(cv::Mat picture);
    void setKetchupGameSingleFryWithKetchup(cv::Mat picture);
    
#endif
    
    double get_max_ketchup_param();
    
    double get_min_considering_ketchup_param();
    
    double calculate_score_increase_for_ketchup_game();
    
    double do_ketchup_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double);
    
    
#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_PRONATION_KETCHUP_H */
