//
// Created by bpiku on 11/27/2018.
//



//The calibration import must always be the first import in order to make use of device-dependent preprocessor directives
#include "calibration.h"
//#include "all_access_vars.h"


//device specific includes/imports
#if MY_OS==ANDROID_OS
    //Below are the imports for the android version
    #include <android/log.h>
    #include <android/native_window_jni.h>
    #include <android/native_window.h>

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    //Below are the imports for the iOS version

#endif


// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cmath>

#include "pronation_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif



// main method that does approximation, returns angle
double analyze_pronation(Mat fgmask, Mat &frame_with_mask, double ratio) {
    distInfo d = get_distance_transform(fgmask);
    double radius = d.dist.at<float>(d.maxLoc.y,d.maxLoc.x);
    //cv::putText(frame_with_mask, std::to_string(radius) , cv::Point(40, 250), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255, 255), 2);
    int top_y = d.maxLoc.y + radius, bottom_y = -1;
    for (int r = 0; r < fgmask.rows; r++) {
        for (int c = 0; c < fgmask.cols; c++) {
            if (d.dist.at<float>(r,c) > 0) {
                bottom_y = r;
                goto end;
            }
        }
    }
    end:
    double param = PARAM_NO_HAND;
    if (bottom_y != -1 && radius > 0.05*std::min(fgmask.cols,fgmask.rows)) {
        param = PARAM_NOT_VALID;
        
        int half_height =  (int)((top_y-bottom_y)/(2));
        int middle_y = top_y-(int)(half_height/2);
        double palm_len = 0.0;
        int start_row = middle_y -1, end_row = middle_y +2; // end row is non-inclusive
        double maxDist = -100000;
        double minDist = 100000;
        for (int r = start_row; r >= start_row && r < end_row; r++) {
            for (int c = 0; c < fgmask.cols; c++) {
                if (r<0 || r >= d.dist.rows || c < 0 || c >= d.dist.cols) {
                    cout << "  Fail  radius: " << std::to_string( (int) radius) << endl;;
                    param = PARAM_NOT_VALID;
                    return param;
                }
                if (d.dist.at<float>(r, c) > 0) {
                    palm_len++;
                    if (displayExtraInfoAmount > 0) {
                        //cv::circle(frame_with_mask, cv::Point(c, r), 1, cv::Scalar(255, 0, 0, 255), 1);
                    }
                    double tempDist = c -d.maxLoc.x;
                    if (tempDist > maxDist) {
                        maxDist = tempDist;
                    }
                    if (tempDist < minDist) {
                        minDist = tempDist;
                    }
                }
            }
        }
        //cout << "Success radius: " << std::to_string(radius);
        palm_len = palm_len/(end_row-start_row);
        if (palm_len > 0) {
            param = palm_len / half_height-1.0;
        } else param = PARAM_NOT_VALID;
        
        if (displayExtraInfoAmount > 0) {
            cv::circle(frame_with_mask, cv::Point(d.maxLoc.x, top_y)/ratio, 5, cv::Scalar(255, 0, 255, 255), 1);
            cv::circle(frame_with_mask, cv::Point(d.maxLoc.x, bottom_y)/ratio, 5, cv::Scalar(255, 0, 0, 255), 1);
            cv::circle(frame_with_mask, cv::Point(d.maxLoc.x - half_height, middle_y)/ratio,5,cv::Scalar(0,0,255, 255));
            cv::circle(frame_with_mask, cv::Point(d.maxLoc.x + half_height, middle_y)/ratio,5,cv::Scalar(0,0,255, 255));
        }
        char buf[125];
        if (abs(minDist) > maxDist) {
            snprintf(buf, sizeof(buf), "P %.2f", param); //std::to_string(param) is the old way, but can't format it
        } else {
            snprintf(buf, sizeof(buf), "S %.2f", param); //std::to_string(param) is the old way, but can't format it
            if (!isParamNoExerciseDone(param)) param = -param;
        }


        if (displayExtraInfoAmount > 1) {
            //cv::putText(frame_with_mask, buf , cv::Point(10, 150), cv::FONT_HERSHEY_SIMPLEX, 6, cv::Scalar(0, 0, 255, 255), 2);
            writeText(frame_with_mask, buf, -1, int(frame_with_mask.rows * 0.70));
            //cv::line(frame_with_mask, cv::Point(d.maxLoc.x-10, top_y-radius*3), Point(d.maxLoc.x+10, top_y-radius*3), Scalar(255,0,255,255),2);
        }


    }
    return param;
}


#ifdef EXTERN_C
}
#endif
