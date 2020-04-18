//
//  fabduction_analysis.hpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/14/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef MOTRACKTHERAPYMOBILE_FABDUCTION_H
#define MOTRACKTHERAPYMOBILE_FABDUCTION_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif

    double analyze_fabduction(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    double analyze_fabduction_unrotated(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    double analyze_fabduction_with_rotation_allowed(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio);
    
#ifdef EXTERN_C
}
#endif

#endif /* MOTRACKTHERAPYMOBILE_FABDUCTION_H */
