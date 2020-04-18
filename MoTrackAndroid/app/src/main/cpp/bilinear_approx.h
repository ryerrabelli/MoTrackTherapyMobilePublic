

#ifndef MOTRACKTHERAPYMOBILE_BILINEAR_APPROX_H
#define MOTRACKTHERAPYMOBILE_BILINEAR_APPROX_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"

#ifdef EXTERN_C
extern "C" {
#endif

//Public variables
extern cv::Mat leftCurveArrow;
extern cv::Mat rightCurveArrow;

//Does main analysis by approximating a bilinear fit to an fgmask. Also draws the feedback arrows.
int bilinear_approx_analysis(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio, bool include_arrows);

//Skeletonizes a distance transform
distInfo plot_diagonalized_max4(distInfo d);
distInfo plot_diagonalized_max5(distInfo d);
distInfo plot_diagonalized_max6(distInfo d);
distInfo plot_diagonalized_max_orig(distInfo d);

// Calculates the circular mean, which is a better "mean" for cyclical values like angles than the standard arithmetic mean
double circular_mean(double angle1, double angle2);
    
// Given a point on the arm, wrist, and hand (palm), calculate the bilinear angle in degrees
int get_bilinear_angle(cv::Point arm_point,cv::Point wrist_point,cv::Point hand_point);

//Adds feedback arrows to the display image for an radial/ulnar deviation game
void addFeedbackArrows( cv::Mat &frame_with_mask, int dir, double ratio, cv::Point wrist, int radius);

#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_BILINEAR_APPROX_H
