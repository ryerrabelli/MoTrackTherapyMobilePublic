//
// Created by Rahul Yerrabelli on 02/21/19.
//

#ifndef MOTRACKTHERAPYMOBILE_DEVIATION_CATAPULT_H
#define MOTRACKTHERAPYMOBILE_DEVIATION_CATAPULT_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif
    
    extern const double CATAPULT_STARTING_PARAM;
    extern double currentCatapultParam;

#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setBullseye(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultMount(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultBeam(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setFieldScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
    
    void setFieldScenery(cv::Mat picture);
    void setBullseye(cv::Mat picture);
    void setCatapultMount(cv::Mat picture);
    void setCatapultBeam(cv::Mat picture);
    
    void setCatapultMountBrokenFront(cv::Mat picture);
    void setCatapultMountBrokenBack(cv::Mat picture);
    void setCatapultMountCracked(cv::Mat picture);



#endif

    double get_max_catapult_param();
    
    double get_min_considering_catapult_param();
    
    double calculate_score_increase_for_catapult_game();
    
    int do_catapult_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double);


#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_DEVIATION_CATAPULT_H
