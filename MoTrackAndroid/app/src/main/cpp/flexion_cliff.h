//
// Created by Rahul Yerrabelli on 3/6/19.
//

#ifndef MOTRACKTHERAPYMOBILE_FLEXION_CLIFF_H
#define MOTRACKTHERAPYMOBILE_FLEXION_CLIFF_H


#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif
    
    extern const double CLIFF_STARTING_PARAM;
    extern double currentCliffParam;
    extern int catapultGameCracksDrawn; //necessary to have this as a different variable than currentCliffParam so that we don't have to draw all the old lines each frame

    
#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setCliffGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCliffGameCliff(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCliffGameGoldNugget(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * -------------------------------------------------------------------------------------------------
     *                                     iOS INTERFACE FUNCTIONS
     * -------------------------------------------------------------------------------------------------
     */
    
    void setCliffGameScenery(cv::Mat cliffImageMat);
    
    void setCliffGameCliff(cv::Mat picture);
    
    void setCliffGameGoldNugget(cv::Mat picture);
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */
    
    //maximum value before it breaks
    double get_max_cliff_param();
    
    //minimum value before it will launch, also the vallue it starts to crack a bit
    double get_min_considering_cliff_param();
    
    double calculate_score_increase_for_cliff_game();
    
    int do_cliff_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int, double);
    
    class flexion_cliff {
        
    };
    
#ifdef EXTERN_C
}
#endif


#endif //MOTRACKTHERAPYMOBILE_FLEXION_CLIFF_H
