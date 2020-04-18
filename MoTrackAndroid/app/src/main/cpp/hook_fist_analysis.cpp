//
//  hfist.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/20/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
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
//#include "Eigen/Dense"

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
//Below are the imports for the iOS version
#include <stdio.h>

#endif


// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cmath>

#include "hook_fist_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    /*
    double analyze_hook_fist(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
     
        vector< vector<Point> > contours; // list of contour points
        vector<Vec4i> hierarchy;
        // find contours
        cv::findContours(fgmask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
     
        double max_area = 0;
        int max_ct_ind = -1;
        for (int i = 0; i < contours.size(); i++) {
            double ct_area = cv::contourArea(contours[i]);
            if (ct_area > max_area) {
                max_area = ct_area;
                max_ct_ind = i;
            }
        }
        if (max_area/(fgmask.rows*fgmask.cols) < 0.05) { // prevent small objects from being detected
            max_ct_ind = -1;
        }
        if (max_ct_ind != -1) {
            for(int i1=0; i1<contours.size(); i1++) {
                for(int i2=0; i2<contours[i1].size(); i2++) {
                    contours[i1][i2] = contours[i1][i2]/ratio;
                }
            }
     
            // create hull array for convex hull points
            vector< vector<int> > hullInds(1); //array of size 1 because will only do this for one contour
            cv::convexHull(Mat(contours[max_ct_ind]), hullInds[0], false); //If input for hull is vector of points will return points. If hull is vector of vector of ints, it will return indices.
     
            //can draw hull, but we would need hull to be a vector< vector<Point> >
            //cv::drawContours(frame_with_mask, hull, 0, CLR_MOTRACK_BLUE_4UC, 4, 8, vector<Vec4i>(), 0, Point());
     
            if (contours[max_ct_ind].size() > 3 && hullInds[0].size() > 3) {
                vector< vector<cv::Vec4i> >convDef(1);
                cv::convexityDefects(contours[max_ct_ind], hullInds[0], convDef[0]);
                for (Vec4i defect : convDef[0]) {
                    if ( abs(contours[0][defect[0]].x) < 2000 && abs(contours[0][defect[1]].x) < 2000 && abs(contours[0][defect[2]].x) < 2000 && abs(contours[0][defect[0]].y) < 2000 && abs(contours[0][defect[1]].y) < 2000 && abs(contours[0][defect[2]].y) < 2000 ) {
                        //can't figure out why, but sometimes defects is randomly super high (positive or negative) values, which can crash the app when trying to draw them with cv::line for example
                        //Example points: [0, 5] [1451292736, 1] [90, 120]; [1451292736, 1] [128, 112] [1470175312, 1]; [128, 112] [0, 0] [0, 0]; [0, 0] [-1362131800, 1] [0, 1441792]; [-1362131800, 1] [220, 108] [0, 5]; [220, 108] [212, 112] [216, 112]; [212, 112] [216, 124] [212, 124]; [216, 124] [1470146280, 1] [0, 0]; [1470146280, 1] [0, 1] [-1, -2097153]; [0, 1] [272, 131112] [360320196, -2147483648]; [0, 0] [0, 0] [0, 0]; [0, 0] [0, 0] [0, 0]; [0, 0] [-2085707364, 1] [512, 1];
     
                        cout << contours[0][defect[0]] << " " << contours[0][defect[1]] << " " << contours[0][defect[2]] << "; " << endl;
     
                        if ( contours[0][defect[0]].x== -2147483648 || contours[0][defect[1]].x== -2147483648 || contours[0][defect[2]].x== -2147483648 || contours[0][defect[0]].y== -2147483648 || contours[0][defect[1]].y== -2147483648 || contours[0][defect[2]].y== -2147483648) {
                            cout << "error" << endl;
     
                        } else {
                            cv::line(frame_with_mask, contours[0][defect[0]], contours[0][defect[2]], Scalar(0,   0, 255, 255), 3);
                            cv::line(frame_with_mask, contours[0][defect[1]], contours[0][defect[2]], Scalar(0, 255, 255, 255), 3);
                            cv::circle(frame_with_mask, contours[0][defect[2]], 5, Scalar(255,255,255,255), 2 );
     
                        }
     
     
                    }
     
                }
                //cout << endl;
            }
     
     
            return NO_EXERCISE_DONE;
     
        } else return NO_EXERCISE_DONE;
    }*/
    

    int firstNonZero(cv::Mat1b& row) {
        int y = 0;
        for (int x = 0; x < row.cols; x++) {
            int rowVal = row.at<int>(y, x);
            if (rowVal != 0) return x;
        }
        return row.cols;
    }
    
    
    double analyze_hook_fist(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        double param = PARAM_NO_HAND;
        
        //check if there is a hand in the screen
        if (cv::countNonZero(fgmask) > 0.05*fgmask.cols*fgmask.rows ) {
            param = PARAM_NOT_VALID;
            
            int stage = 0;
            
            vector<int> rowWds;
            rowWds.reserve(fgmask.rows);
            vector<float> d_dy; //derivative of countNonZero(row) with respective to vertical position
            d_dy.reserve(fgmask.rows-1);
            //save prevRowWd to reduce the number of times countNonZero is called
            int prevRowWd = 0; //value of 0 is unused placeholder
            for (int row_it = 0; row_it < fgmask.rows; row_it++) {
                Mat1b row = fgmask.row(row_it);
                int nextRowWd = cv::countNonZero(row);
                rowWds.push_back( nextRowWd );
                if (row_it != 0) {
                    d_dy.push_back( (nextRowWd-prevRowWd)/1.0 );
                }
                prevRowWd = nextRowWd;
            }
            
            int line1 = -1;
            int line2 = -1;
            
            //NOTE: There are n-1 differences between rows (thus n-1 is the length of the derivative vector) if there are n rows
            for (int diff_row_it = 0; diff_row_it < fgmask.rows-1; diff_row_it++) {
                double slope = d_dy[diff_row_it];
                
                //int rowWd = rowWds[diff_row_it];
                //cv::line(frame_with_mask, cv::Point(0, diff_row_it)/ratio, cv::Point( rowWd, diff_row_it)/ratio, CLR_MOTRACK_BLUE_4UC, 1 );
                
                if (stage % 2 == 0) {
                    if (slope > 0) {
                        if (stage < 2) line1 = diff_row_it;
                        if (displayExtraInfoAmount > 1) {
                            cv::line(frame_with_mask, cv::Point(0, diff_row_it+0.5)/ratio, cv::Point( fgmask.cols, diff_row_it+0.5)/ratio, Scalar(0,255,0,255), (stage < 2 ? 5 : 1) );
                        } else if (stage >= 2) {
                            break;
                        }
                        stage++;
                    }
                    
                } else if (stage % 2 == 1) {
                    if (slope < 0) {
                        if (stage < 2) line2 = diff_row_it;
                        if (displayExtraInfoAmount > 1) {
                            cv::line(frame_with_mask, cv::Point(0, diff_row_it+0.5)/ratio, cv::Point( fgmask.cols, diff_row_it+0.5)/ratio, Scalar(255,0,0,255), (stage < 2 ? 5 : 1) );
                        } else if (stage >= 2) {
                            break;
                        }
                        stage++;
                    }
                }
            }
            
            if (line1 != -1 && line2 != -1) {
                //NOTE: line1 and line2 will refer to the first part
                //cout << line1 << " " << line2 << endl;
                
                int rect1Start = line1;
                int rect1Ht = (line2-line1)*1.5;
                int rect2Start = rect1Start+rect1Ht;
                int rect2Ht = rect1Ht;
                
                Rect    knuckles(0, rect1Start, fgmask.cols, rect1Ht);
                Rect comparePart(0, rect2Start, fgmask.cols, rect2Ht);
                if (comparePart.y+comparePart.height < fgmask.rows) {
                    //cout << knuckles << endl;
                    int knuckleArea = cv::countNonZero(fgmask(knuckles));
                    int compareArea = cv::countNonZero(fgmask(comparePart));
                    param = knuckleArea * 1.0/compareArea;
                    
                    
                    if (displayExtraInfoAmount > 0) {
                        cout << param << endl;
                        
                        Rect    knucklesDraw(0, rect1Start/ratio, fgmask.cols/ratio, rect1Ht/ratio );
                        Rect comparePartDraw(0, rect2Start/ratio, fgmask.cols/ratio, rect2Ht/ratio );
                        cv::rectangle(frame_with_mask,    knucklesDraw, Scalar(0,255,0,255), 10);
                        cv::rectangle(frame_with_mask, comparePartDraw, Scalar(255,0,0,255), 10);
                    }
                }
                
            }
            
            if (displayExtraInfoAmount > 1) {
                for (int row_it = 0; row_it < fgmask.rows; row_it++) {
                    
                    cv::Scalar color;
                    int thickness = 1;
                    
                    Mat1b row = fgmask.row(row_it);
                    int rowNonZero = cv::countNonZero(row);
                    
                    if (rowNonZero>0) {
                        if (row_it != 0 && row_it != fgmask.rows-1) {
                            Mat1b row_1 = fgmask.row(row_it-1);
                            int rowNonZero_1 = cv::countNonZero(row_1);
                            
                            Mat1b row1 = fgmask.row(row_it+1);
                            int rowNonZero1 = cv::countNonZero(row1);
                            
                            double slope = (rowNonZero1-rowNonZero_1)/2.0;
                            if (stage == 0) {
                                if (slope > 1) stage++;
                                
                            } else if (stage == 1) {
                                if (slope < 0) {
                                    stage++;
                                    cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point( fgmask.cols, row_it)/ratio, Scalar(255,255,255,255), thickness*3 );
                                }
                            }
                            
                            
                            color = CLR_MOTRACK_BLUE_4UC;
                            cv::line(frame_with_mask, cv::Point(fgmask.cols/2, row_it)/ratio, cv::Point( fgmask.cols/2+10.0*slope, row_it)/ratio, color, thickness );
                            
                            //cout << rowNonZero_1 << " " << rowNonZero << " " << rowNonZero1 << endl;
                            if (rowNonZero > rowNonZero_1 && rowNonZero > rowNonZero1) {
                                thickness = 3;
                                color = Scalar(0,255,0,255); //green
                                cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point(rowNonZero, row_it)/ratio, color, thickness );
                                
                            } else if ( (rowNonZero >= rowNonZero_1 && rowNonZero > rowNonZero1) || (rowNonZero > rowNonZero_1 && rowNonZero >= rowNonZero1)) {
                                color = Scalar(127,255,127,255); //light green
                                cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point(rowNonZero, row_it)/ratio, color, thickness );
                                
                            } else if ( rowNonZero == rowNonZero_1 && rowNonZero == rowNonZero1) {
                                color = Scalar(255,255,127,255); //light yellow
                                cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point(rowNonZero, row_it)/ratio, color, thickness );
                                
                            } else if (rowNonZero < rowNonZero_1 && rowNonZero < rowNonZero1) {
                                color = Scalar(255,0,0,255); //red
                                cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point(rowNonZero, row_it)/ratio, color, thickness );
                                
                            } else if ( (rowNonZero <= rowNonZero_1 && rowNonZero < rowNonZero1) || (rowNonZero < rowNonZero_1 && rowNonZero <= rowNonZero1) ) {
                                thickness = 3;
                                color = Scalar(255,127,127,255); //light red
                                cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point(rowNonZero, row_it)/ratio, color, thickness );
                                
                            } else {
                                color = CLR_MOTRACK_BLUE_4UC;
                                cv::line(frame_with_mask, cv::Point(0, row_it)/ratio, cv::Point(rowNonZero, row_it)/ratio, color, thickness );
                                
                            }
                            
                        } else color = CLR_MOTRACK_BLUE_4UC;
                        
                    }
                    
                }
            }
            
        }
        
        return param;

    }
    
    
    
    
#ifdef EXTERN_C
}
#endif
