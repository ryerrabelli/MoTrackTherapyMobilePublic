//
//  hfist.hpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/20/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_HOOKED_FIST_ANALYSIS_H
#define MOTRACKTHERAPYMOBILE_HOOKED_FIST_ANALYSIS_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif

    double analyze_hook_fist(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    
#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_HOOKED_FIST_ANALYSIS_H */
