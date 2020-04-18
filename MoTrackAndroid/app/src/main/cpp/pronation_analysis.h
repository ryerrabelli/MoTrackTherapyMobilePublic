//
// Created by bpiku on 11/27/2018.
//

#ifndef MOTRACKTHERAPYMOBILE_ROTATION_H
#define MOTRACKTHERAPYMOBILE_ROTATION_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif
    

    double analyze_pronation(cv::Mat fgmask, cv::Mat &frame_with_mask, double ratio);


#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_ROTATION_H
