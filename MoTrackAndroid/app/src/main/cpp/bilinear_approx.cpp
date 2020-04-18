//
// Created by ? on ?.
//


//The calibration import must always be the first import in order to make use of device-dependent preprocessor directives
#include "calibration.h"
//#include "all_access_vars.h"

//device specific includes/imports
#if MY_OS==ANDROID_OS
    //Below are the imports for the android version

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    //Below are the imports for the iOS version

#endif


// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cmath>
#include "fist_analysis.h"
#include "bilinear_approx.h"



using namespace cv;
using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif

// Public variables
cv::Mat leftCurveArrow;
cv::Mat rightCurveArrow;


// main method that does approximation, returns angle
int bilinear_approx_analysis(Mat& fgmask, Mat &frame_with_mask, int dir, double ratio, bool include_arrows) {
    int drawing_thickness = 1;
    if (displayExtraInfoAmount > 2) { drawing_thickness = 3; }

    int angle = PARAM_NOT_VALID;
    if (cv::countNonZero(fgmask)*1.0/(fgmask.cols*fgmask.rows) < 0.05) {
        angle = PARAM_NO_HAND;
        
    } else {
        //Get distance transform
        distInfo dres = get_distance_transform(fgmask); //dres stands for d results
        int radius = (int) (dres.dist.at<float>(dres.maxLoc.y, dres.maxLoc.x));
        cv::normalize(dres.dist, dres.dist, 0, 1.0, cv::NORM_MINMAX);
        
        //To here from start of the program is between 1-1.5 picoseconds (regardless of whether 0% or 100% of the screen was within mask) on Rahul's Android
        
        //Below code can be used to time parts of the method
        //chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        
        //Original version part took from 0.1 psec (if mask was empty ) to avg ~20psec and max 40psec if the mask was super heavy
        //Skeletonize the distance form
        //dres = plot_diagonalized_max_orig(dres); //original before Feb 3rd-5th, 2019 optimization
        //dres = plot_diagonalized_max1(dres); //for testing, not an actual implementation
        //dres = plot_diagonalized_max2(dres); //for testing, not an actual implementation
        //dres = plot_diagonalized_max3(dres); //for testing, not an actual implementation
        //dres = plot_diagonalized_max4(dres); //Made it better while being functionally equivalent. Now let's try to do some shortcuts to make it even faster.
        //dres = plot_diagonalized_max5(dres); //0.08 -> 0.09 psec normal, actually made it worse, so go back to #4
        dres = plot_diagonalized_max6(dres); //this is the fastest, at 1/3 of the time as orig
        
        //Below code can be used to time parts of the method
        //chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
        //auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        //cout << time_span.count()/1000.0 << "ms" << endl; //*/
        
        
        //double max_x_loc, max_y_loc;
        double max_dist_mag = 0, this_max_dist_mag = 0, max_dist_x = 0, max_dist_y = 0;
        int max_y_loc = 0, max_y = 0; //get the highest point on skeletonization
        for (int i = 0; i < dres.xdata.size(); i++) {
            int this_y = dres.ydata.at(i);
            int this_x = dres.xdata.at(i);
            // Find max loc, the highest point of the skeletonization
            if (this_y > max_y) {
                max_y_loc = i;
                max_y = this_y;
            }
            
            //this_max_dist_mag = (dres.dist.at<float>(this_y, this_x)) / ( (std::abs(frame_with_mask.rows-this_y)) * (std::abs(frame_with_mask.cols-this_x)));
            this_max_dist_mag = (dres.dist.at<float>(this_y, this_x));
            //this_max_dist_mag = (dres.dist.at<float>(this_y, this_x)) / ( (std::abs(frame_with_mask.rows-this_y)) );
            if (this_max_dist_mag > max_dist_mag) {
                max_dist_mag = this_max_dist_mag;
                max_dist_x = this_x;
                max_dist_y = this_y;
            }
            
            
            //draw skeletonization for testing
            if (displayExtraInfoAmount > 2) {
                //for unknown reasons, this doesn't work very well, but the circle method does work properly instead
                //frame_with_mask.at<Scalar>(dres.xdata.at(i),dres.ydata.at(i)) = cv::Scalar(0,255,0,255);
                /*for (int i = 0; i < dres.xdata.size(); i++) {
                 cv::circle(frame_with_mask,
                 cv::Point(dres.xdata.at(i) / ratio, dres.ydata.at(i) / ratio), int(1/ratio),
                 cv::Scalar(0, 255, 0, 255));
                 }*/
                //draw circle representing palm of hand if hand prone (i.e. windshield game) or side of palm of hand if hand neutral (i.e. flexion game)
                cv::circle(frame_with_mask, dres.maxLoc/ratio, int(radius/ratio), cv::Scalar(0, 0, 255, 255), int(1/ratio));
            }
        }
        
        //decrease radius to accomodate for the fact that we might have had to move it accidentally from the forearm to palm
        radius = radius * max_dist_mag/dres.dist.at<float>(dres.maxLoc.y, dres.maxLoc.x);
        //dres.maxLoc = cv::Point(max_dist_x,max_dist_y);
        dres.maxLoc.x = max_dist_x;
        dres.maxLoc.y = max_dist_y;
        
        
        
        
        // find intersection of circle and line
        double min_dist = 100000;
        int closest_x = 0, closest_y = 0;
        /*if (radius > 150) {
         return 0;
         }*/
        /*
         for (double i = 0;
         i < 3.1415926; i += 0.0174533) { // iterate from 0 degrees to 180 by 1 degree (in radians)
         int x = (int) (radius * cos(i) + dres.maxLoc.x);
         int y = (int) (radius * sin(i) + dres.maxLoc.y);
         for (int j = 0; j < dres.xdata.size(); j++) {
         double dist = pow(x - dres.xdata.at(j), 2) + pow(y - dres.ydata.at(j), 2);
         if (dist < min_dist) {
         min_dist = dist;
         closest_x = x;
         closest_y = y;
         //if (dist < 1) {
         //    goto end;
         // distance is small enough, get out of for loop - intersection has been found
         //}
         }
         }
         }*/
        
        for (int j = 0; j < dres.xdata.size(); j++) {
            double min_dist_this_cycle = 100000;
            // iterate from 0 degrees to 180 by 1 degree (in radians)
            for (double i = 0; i < 3.1415926; i += 0.0174533) {
                int x = (int) (radius * cos(i) + dres.maxLoc.x);
                int y = (int) (radius * sin(i) + dres.maxLoc.y);
                double dist = pow(x - dres.xdata.at(j), 2) + pow(y - dres.ydata.at(j), 2);
                
                if (dist < min_dist_this_cycle) {
                    min_dist_this_cycle = dist;
                    if (dist < min_dist) {
                        min_dist = dist;
                        closest_x = x;
                        closest_y = y;
                    }
                }
                
            }
            
            // If min_dist_this_cycle after having gotten small enough, then that means we already found the first intersection, aka the correct intersection
            // This way, we can avoid getting to later intersections which the code might incorrectly think is the right one
            if (min_dist < 5 && min_dist_this_cycle>min_dist*3) {
                goto foundIntersection;
            }
        }
        
        foundIntersection: //label syntax, see https://www.tutorialspoint.com/cplusplus/cpp_goto_statement.htm
        cv::Point wrist(closest_x, closest_y); //is in fgmask coordinate space
        if (max_y_loc < dres.ydata.size()) {
            cv::Point bottomArm(dres.xdata.at(max_y_loc), dres.ydata.at(max_y_loc)); //is in fgmask coordinate space
            // cv::Point palm() is dres.maxLoc
            angle = get_bilinear_angle(bottomArm, wrist, dres.maxLoc);
            //frame_with_mask = writeScoreFeedback(frame_with_mask,angle);
            if (displayExtraInfoAmount > 1) {
                if (displayExtraInfoAmount > 2) {
                    cout << "bottomArm: (" <<  bottomArm.x << ", " << bottomArm.y << "), wrist: (" << wrist.x << ", " << wrist.y << "), maxLoc: (" << dres.maxLoc.x << ", " << dres.maxLoc.y << ") with r=" << radius << " => ";
                }
                
                cv::circle(frame_with_mask, wrist/ratio, 20, cv::Scalar(0, 0, 255, 255));
                cv::line(frame_with_mask, wrist/ratio, bottomArm/ratio, cv::Scalar(255, 0, 0, 255), drawing_thickness);
                cv::line(frame_with_mask, dres.maxLoc/ratio, wrist/ratio,  cv::Scalar(255, 0, 0, 255), drawing_thickness);
            }
            if (angle < -100) {
                angle = PARAM_NOT_VALID;
                
            } else if (angle == 180) { //this happens sometimes, probably a bug
                angle = angle;
            }
            
            //cv::putText(frame_with_mask,"Angle: " + std::to_string(angle), cv::Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,0,0, 255), 3);
            //writeText(frame_with_mask,"Angle: " + std::to_string(angle),-1,int(frame_with_mask.rows*0.35));
        }
        cv::Point toppalm(dres.maxLoc.x, dres.maxLoc.y-radius); //is in fgmask coordinate space, is top of palm where knuckle meet hand
        
        if (include_arrows) {
            addFeedbackArrows(frame_with_mask, dir, ratio, toppalm, radius);
        }
    }
    
    if (displayExtraInfoAmount > 1) cout << angle << endl;

    return angle;//frame_with_mask;
}


distInfo plot_diagonalized_max4(distInfo d) {
    //double angles = -M_PI/2;//-1.57079; // 90 degrees in radians
    int height = d.dist.rows;
    int width = d.dist.cols;
    vector<float> sharpAngles;
    vector<float> angles;
    angles.push_back(-M_PI/2);//-1.57079; // 90 degrees in radian
    d.xdata.push_back(int(width/2));
    d.ydata.push_back(height-1);
    int it = 0;
    double maxY = 0, maxX = 0;
    while (it < 1000) {
        float maxVal = 0;
        float slice_sum = 0;
        float lastx = d.xdata.back();
        float lasty = d.ydata.back();
        float lastAng = angles.back();

        if ((abs(lastAng) >= M_PI/4) && (abs(lastAng) <= M_PI*3/4)) {
            // all x points within the image (from 0,1,2,3,... to width)
            for (int x = 0; x < width; x++) {
                int y = (int)(round(1.0*tan(lastAng+M_PI/2)*(x-lastx) + lasty));
                if (y >= 0 && y < height) {
                    if ((it < 5) ||  ((x-lastx)*(x-lastx) + (y-lasty)*(y-lasty) < 1.5)) {
                        float distAt = d.dist.at<float>(y, x);
                        slice_sum += distAt;
                        if (distAt > maxVal) {
                            maxVal = distAt;
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                int x = (int)(round(1.0/tan(lastAng+M_PI/2)*(y-lasty) + lastx));
                if (x >= 0 && x < width) {
                    if ((it < 5) ||  ((x-lastx)*(x-lastx) + (y-lasty)*(y-lasty) < 1.5)) {
                        float distAt = d.dist.at<float>(y, x);
                        slice_sum += distAt;
                        if (distAt > maxVal) {
                            maxVal = distAt;
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
            }
            if (slice_sum <= 0.0001) {
                break;
            }
        }

        it++;
        d.xdata.back() = (maxX);
        d.ydata.back() = (maxY);
        if (it >= 3) {
            double dely = d.ydata.back() -d.ydata.end()[-2];
            double delx = d.xdata.back() -d.xdata.end()[-2];
            sharpAngles.push_back( atan2( dely, delx ) );
            if (sharpAngles.size() > 6) {
                angles.push_back(circular_mean(angles.back(),sharpAngles.back()));
            } else {
                angles.push_back(angles.back());
            }
        } else {
            angles.push_back(angles.back());
        }
        //make a guess on the next values
        d.xdata.push_back( d.xdata.back() + cos(angles.back()) );
        d.ydata.push_back( d.ydata.back() + sin(angles.back()) );
        //dist.at<float>(maxY, maxX) = 255;//cv::Scalar(0,0,255, 255);
    }
    return d;
}

distInfo plot_diagonalized_max5(distInfo d) {
    //double angles = -M_PI/2;//-1.57079; // 90 degrees in radians
    int height = d.dist.rows;
    int width = d.dist.cols;
    vector<float> sharpAngles;
    vector<float> angles;
    float lastAng = -1.57079; //-1.57079, -pi/2; // 90 degrees in radian
    angles.push_back(lastAng);
    //d.xdata.push_back(int(width/2));
    //d.ydata.push_back(height-1);

    float xdataGuess = int(width/2);
    float ydataGuess = height-1;

    int it = 0;
    double maxY = 0, maxX = 0;
    while (it < 1000) {
        float maxVal = 0;
        float slice_sum = 0;

        if ((abs(lastAng) >= M_PI/4) && (abs(lastAng) <= M_PI*3/4)) {
            // all x points within the image (from 0,1,2,3,... to width)
            for (int x = 0; x < width; x++) {
                int y = (int)(round(1.0*tan(lastAng+M_PI/2)*(x-xdataGuess) + ydataGuess));
                if (y >= 0 && y < height) {
                    if ((it < 5) ||  ((x-xdataGuess)*(x-xdataGuess) + (y-ydataGuess)*(y-ydataGuess) < 1.5)) {
                        float distAt = d.dist.at<float>(y, x);
                        slice_sum += distAt;
                        if (distAt > maxVal) {
                            maxVal = distAt;
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                int x = (int)(round(1.0/tan(lastAng+M_PI/2)*(y-ydataGuess) + xdataGuess));
                if (x >= 0 && x < width) {
                    if ((it < 5) ||  ((x-xdataGuess)*(x-xdataGuess) + (y-ydataGuess)*(y-ydataGuess) < 1.5)) {
                        float distAt = d.dist.at<float>(y, x);
                        slice_sum += distAt;
                        if (distAt > maxVal) {
                            maxVal = distAt;
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
            }
            if (slice_sum <= 0.0001) {
                break;
            }
        }

        it++;
        //maxX, maxY now represent the corrected value of the next value within d.xdata and d.ydata
        //d.xdata.back() = (maxX);
        //d.ydata.back() = (maxY);
        d.xdata.push_back(maxX);
        d.ydata.push_back(maxX);

        if (it >= 3) {
            double dely = maxY -d.ydata.end()[-2];
            double delx = maxX -d.xdata.end()[-2];
            sharpAngles.push_back( atan2( dely, delx ) );
            if (sharpAngles.size() > 6) {
                lastAng = circular_mean(angles.back(),sharpAngles.back());
                angles.push_back(lastAng);
            } else {
                angles.push_back(lastAng); //repeat same value
            }
        } else {
            angles.push_back(lastAng); //repeat same value
        }
        //make a guess on the next values
        xdataGuess = d.xdata.back() + cos(lastAng);
        ydataGuess = d.ydata.back() + cos(lastAng);
        //d.xdata.push_back( d.xdata.back() + cos(angles.back()) );
        //d.ydata.push_back( d.ydata.back() + sin(angles.back()) );
    }
    return d;
}

distInfo plot_diagonalized_max6(distInfo d) {
    //double angles = -M_PI/2;//-1.57079; // 90 degrees in radians
    int height = d.dist.rows;
    int width = d.dist.cols;
    vector<float> sharpAngles;
    vector<float> angles;
    angles.push_back(-M_PI/2);//-1.57079; // 90 degrees in radian
    d.xdata.push_back(int(width/2));
    d.ydata.push_back(height-1);
    int it = 0;
    double maxY = 0, maxX = 0;
    while (it < 1000) {
        float maxVal = 0;
        float slice_sum = 0;
        float lastx = d.xdata.back();
        float lasty = d.ydata.back();
        float lastAng = angles.back();

        if ((abs(lastAng) >= M_PI/4) && (abs(lastAng) <= M_PI*3/4)) {
            // all x points within the image (from 0,1,2,3,... to width)
            int x    = ( (it<5 || lastx-10 < 0    ) ? 0     : lastx-10 ); //start x value
            int endX = ( (it<5 || lastx+10 > width) ? width : lastx+10 ); //end x value
            while (x < endX) {
                int y = (int)(round(1.0*tan(lastAng+M_PI/2)*(x-lastx) + lasty));
                if (y >= 0 && y < height) {
                    if ((it < 5) ||  ((x-lastx)*(x-lastx) + (y-lasty)*(y-lasty) < 1.5)) {
                        float distAt = d.dist.at<float>(y, x);
                        slice_sum += distAt;
                        if (distAt > maxVal) {
                            maxVal = distAt;
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
                x++;
            }
        } else {
            int y    = ( (it<5 || lasty-10 < 0     ) ? 0      : lasty-10 ); //start y value
            int endY = ( (it<5 || lasty+10 > height) ? height : lasty+10 ); //end y value
            while (y < endY) {
                int x = (int)(round(1.0/tan(lastAng+M_PI/2)*(y-lasty) + lastx));
                if (x >= 0 && x < width) {
                    if ((it < 5) ||  ((x-lastx)*(x-lastx) + (y-lasty)*(y-lasty) < 1.5)) {
                        float distAt = d.dist.at<float>(y, x);
                        slice_sum += distAt;
                        if (distAt > maxVal) {
                            maxVal = distAt;
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
                y++;
            }
            if (slice_sum <= 0.0001) {
                break;
            }
        }

        it++;
        d.xdata.back() = (maxX);
        d.ydata.back() = (maxY);
        if (it >= 3) {
            double dely = d.ydata.back() -d.ydata.end()[-2];
            double delx = d.xdata.back() -d.xdata.end()[-2];
            sharpAngles.push_back( atan2( dely, delx ) );
            if (sharpAngles.size() > 6) {
                angles.push_back(circular_mean(angles.back(),sharpAngles.back()));
            } else {
                angles.push_back(angles.back());
            }
        } else {
            angles.push_back(angles.back());
        }
        //make a guess on the next values
        d.xdata.push_back( d.xdata.back() + cos(angles.back()) );
        d.ydata.push_back( d.ydata.back() + sin(angles.back()) );
        //dist.at<float>(maxY, maxX) = 255;//cv::Scalar(0,0,255, 255);
    }
    return d;
}

distInfo plot_diagonalized_max_orig(distInfo d) {
    //double angles = -M_PI/2;//-1.57079; // 90 degrees in radians
    int height = d.dist.rows;
    int width = d.dist.cols;
    vector<double> sharpAngles;
    vector<double> angles;
    angles.push_back(-M_PI/2);//-1.57079; // 90 degrees in radian
    d.xdata.push_back(int(width/2));
    d.ydata.push_back(height-1);
    int it = 0;
    double maxY = 0, maxX = 0;
    while (it < 1000) {
        float maxVal = 0;
        float slice_sum = 0;
        if ((abs(angles.back()) >= M_PI/4) && (abs(angles.back()) <= M_PI*3/4)) {
            // all x points within the image (from 0,1,2,3,... to width)
            for (int x = 0; x < width; x++) {
                int y = (int)(round(tan(angles.back()+M_PI/2)*(x-d.xdata.back()) + d.ydata.back()));
                if (y >= 0 && y < height) {
                    if ((it < 5) || ((pow(x-d.xdata.back(),2)+pow(y-d.ydata.back(),2)) < 1.5)) {
                        slice_sum += d.dist.at<float>(y, x);
                        if (d.dist.at<float>(y, x) > maxVal) {
                            maxVal = d.dist.at<float>(y, x);
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                int x = (int)(round(1.0/tan(angles.back()+M_PI/2)*(y-d.ydata.back()) + d.xdata.back()));
                if (x >= 0 && x < width) {
                    if ((it < 5) || ((pow(x-d.xdata.back(),2)+pow(y-d.ydata.back(),2)) < 1.5)) {
                        slice_sum += d.dist.at<float>(y, x);
                        if (d.dist.at<float>(y, x) > maxVal) {
                            maxVal = d.dist.at<float>(y, x);
                            maxY = y;
                            maxX = x;
                        }
                    }
                }
            }
            if (slice_sum <= 0.0001) {
                break;
            }
        }

        it++;
        d.xdata.back() = (maxX);
        d.ydata.back() = (maxY);
        if (it >= 3) {
            double dely = d.ydata.back() -d.ydata.end()[-2];
            double delx = d.xdata.back() -d.xdata.end()[-2];
            sharpAngles.push_back( atan2( dely, delx ) );
            if (sharpAngles.size() > 6) {
                angles.push_back(circular_mean(angles.back(),sharpAngles.back()));
            } else {
                angles.push_back(angles.back());
            }
        } else {
            angles.push_back(angles.back());
        }
        //double cos_degree = cos(angles.back());
        //double sin_degree = sin(angles.back());
        //make a guess on the next values
        d.xdata.push_back( d.xdata.back() + cos(angles.back()) );
        d.ydata.push_back( d.ydata.back() + sin(angles.back()) );
        //dist.at<float>(maxY, maxX) = 255;//cv::Scalar(0,0,255, 255);
    }
    return d;
}

// Calculates the circular mean, which is a better "mean" for cyclical values like angles than the standard arithmetic mean
// Input and output in radians; 0 and pi are different, but 0 and 2pi are the same
double circular_mean(double angle1, double angle2) {
    double sinSum = sin(angle1) + sin(angle2);
    double cosSum = cos(angle1) + cos(angle2);
    return atan2(sinSum,cosSum);
}


// Given a point on the arm, wrist, and hand (palm), calculate the bilinear angle in degrees
int get_bilinear_angle(cv::Point arm_point,cv::Point wrist_point,cv::Point hand_point) {
    double m1 = (double)((arm_point.x-wrist_point.x))/(arm_point.y-wrist_point.y);
    double m2 = (double)((wrist_point.x-hand_point.x))/(wrist_point.y-hand_point.y);
    double angle = atan(m1)-atan(m2);
    return (int)(angle*180/M_PI);
}


//Adds feedback arrows to the display image for an radial/ulnar deviation game
//Note the input wrist point is in coordinates of the fgmask, not in frame_with_mask coordinates (divide by ratio
void addFeedbackArrows(cv::Mat &frame_with_mask, int dir, double ratio, cv::Point wrist, int radius) {
    if (radius > 0) {
        if (leftCurveArrow.data == NULL) {
            if (rightCurveArrow.data == NULL) {
                //can't draw feedback arrows. Abort!
                return;
            } else {
                cv::flip(rightCurveArrow, leftCurveArrow, +1); //copy rightCurveArrow from the left version, flipping horizontally
            }
        } else if (rightCurveArrow.data == NULL) {
            cv::flip(leftCurveArrow, rightCurveArrow, +1); //copy leftCurveArrow from the right version, flipping horizontally
        } else {
            //everything is dandy, no need to do any flipping/cancelling
        }
        
        //Now, actually get to adding the feedback arrows
        if ( (dir == 1 && whichHand == Hand::RIGHT)  || (dir == 0 && whichHand == Hand::LEFT)) { // left arrow
            // below part copied and refactored from fist.cpp
            Mat leftCurveArrowResized;
            // Set width to the diameter (twice the radius). Then increased it by a bit more to be 3x so it didn't look exactly equal to diameter, which looked too artificial.
            cv::resize(leftCurveArrow, leftCurveArrowResized,
                       cv::Size((int) (2 * radius/ratio), (int) (2 * radius/ratio * leftCurveArrow.rows / leftCurveArrow.cols)),
                       0, 0, INTER_LINEAR);
            
            int width_arrow = leftCurveArrowResized.cols;
            int height_arrow = leftCurveArrowResized.rows;
            int start_x_arrow = wrist.x/ratio - (int) (width_arrow * 1.5);
            int start_y_arrow = wrist.y/ratio;//top_y - height_bottle;
            // if (width_arrow + start_x_arrow < frame_with_mask.cols &&
            //    height_arrow + start_y_arrow < frame_with_mask.rows &&
            //   start_x_arrow > 0 && start_y_arrow > 0) {
            //copyWithPixelTransparencyUsingAlpha(frame_with_mask, leftCurveArrowResized, start_x_arrow, start_y_arrow, width_arrow, height_arrow, 250);
            drawImageOptimized(frame_with_mask, leftCurveArrowResized, start_x_arrow, start_y_arrow);

            //}
            // just arrow
            //cv::arrowedLine(frame_with_mask, wrist, cv::Point(wrist.x+200,wrist.y), cv::Scalar(0,255, 0, 255), 25, 8,0, 0.1);
            
            
        } else if ( (dir == 0 && whichHand == Hand::RIGHT)  || (dir == 1 && whichHand == Hand::LEFT)) { // right arrow
            if (radius/ratio > 0) {
                // below part copied and refactored from fist.cpp
                Mat rightCurveArrowResized;

                // Set width to the diameter (twice the radius). Then increased it by a bit more to be 3x so it didn't look exactly equal to diameter, which looked too artificial.
                cv::resize(rightCurveArrow, rightCurveArrowResized,
                           cv::Size((int) (2 * radius / ratio),
                                    (int) (2 * radius / ratio * rightCurveArrow.rows /
                                           rightCurveArrow.cols)),
                           0, 0, INTER_LINEAR);

                int width_arrow = rightCurveArrowResized.cols;
                int height_arrow = rightCurveArrowResized.rows;
                int start_x_arrow = wrist.x / ratio + (int) (width_arrow * 0.5);
                int start_y_arrow = wrist.y / ratio;//top_y - height_bottle;
                /*if (start_x_arrow < frame_with_mask.cols && start_y_arrow < frame_with_mask.rows) {
                 if (width_arrow + start_x_arrow >= frame_with_mask.cols) {
                 rightCurveArrowResized = rightCurveArrowResized.colRange(0,
                 frame_with_mask.cols -
                 start_x_arrow - 1);
                 }
                 if (height_arrow + start_y_arrow > frame_with_mask.rows) {
                 rightCurveArrowResized = rightCurveArrowResized.rowRange(0,
                 frame_with_mask.rows -
                 start_y_arrow - 1);
                 }

                 //if (width_arrow + start_x_arrow < frame_with_mask.cols &&
                 //   height_arrow + start_y_arrow < frame_with_mask.rows &&
                 if (start_x_arrow > 0 && start_y_arrow > 0) {
                 copyWithPixelTransparencyUsingAlpha(frame_with_mask, rightCurveArrowResized,
                 start_x_arrow, start_y_arrow, width_arrow,
                 height_arrow, 250);
                 }
                 }*/
                //copyWithPixelTransparencyUsingAlpha(frame_with_mask, rightCurveArrowResized, start_x_arrow, start_y_arrow, width_arrow, height_arrow, 250);
                drawImageOptimized(frame_with_mask, rightCurveArrowResized, start_x_arrow, start_y_arrow);
                
                // just arrow
                //cv::arrowedLine(frame_with_mask, wrist, cv::Point(wrist.x-200,wrist.y), cv::Scalar(0,255, 0, 255), 25, 8,0, 0.1);
            }
        }
    }
}

#ifdef EXTERN_C
}
#endif
