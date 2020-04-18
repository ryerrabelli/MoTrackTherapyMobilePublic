//
//  pronation_ketchup.cpp
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 3/11/19.
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

#include "pronation_ketchup.h"
#include "pronation_analysis.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    //const variables that hold coordinates are measured as percentage of frame dimensions
    const double CAP_RIGHT_X_LOC_WITHIN_KETCHUP_BOTTLE = 31.0/450;
    const double KETCHUP_PILE_RADIUS_TO_PLATE_RATIO = 0.3/2; //this is for the max param, divide by 2 to go from diameter to radius
    const double KETCHUP_PILE_EXTRA_RADIUS_EXPLODE_RATIO = 0.5;
    const double SINGLEFRY_CENTER_Y_LOC_WITHIN_PLATEWITHFRIES = 1.0/3;
    const double KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES = 0.75;
    
    const double KETCHUP_STARTING_PARAM = 0.0;
    double currentKetchupParam = KETCHUP_STARTING_PARAM;
    double ketchupParamIncreaseAmt = 0;
    cv::Mat ketchupGameScenery;
    cv::Mat plateWithFries;
    cv::Mat ketchupBottle;
    cv::Mat ketchupBottleCap;
    cv::Mat ketchupGameSingleFry;
    
    cv::Mat ketchupGameSceneryOrig;
    cv::Mat plateWithFries1Orig;
    cv::Mat plateWithFries2Orig;
    cv::Mat ketchupBottleOrig;
    cv::Mat ketchupBottleCapOrig;
    cv::Mat ketchupGameSingleFryWithoutKetchupOrig;
    cv::Mat ketchupGameSingleFryWithKetchupOrig;
    
    bool ketchupGameImagesInitialized = false;


#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameScenery(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, ketchupGameSceneryOrig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGamePlateWithFries1(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, plateWithFries1Orig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGamePlateWithFries2(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, plateWithFries2Orig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameKetchupBottle(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, ketchupBottleOrig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameKetchupBottleCap(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, ketchupBottleCapOrig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameSingleFryWithoutKetchup(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, ketchupGameSingleFryWithoutKetchupOrig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setKetchupGameSingleFryWithKetchup(JNIEnv *env, jobject, long addr) {
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::Mat picture = *matAddress;
            if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
            cv::cvtColor(picture, ketchupGameSingleFryWithKetchupOrig, cv::COLOR_BGRA2RGBA);
            ketchupGameImagesInitialized = false;
        }
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * ---------------------------------------------------------------------------
     *                         iOS INTERFACE FUNCTIONS
     * ---------------------------------------------------------------------------
     */
    void setKetchupGameScenery(cv::Mat picture) {
        if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        ketchupGameSceneryOrig = picture;
    }
    
    void setKetchupGamePlateWithFries1(cv::Mat picture) {
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        plateWithFries1Orig = picture;
    }
    
    void setKetchupGamePlateWithFries2(cv::Mat picture) {
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        plateWithFries2Orig = picture;
    }
    
    void setKetchupGameKetchupBottle(cv::Mat picture) {
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        ketchupBottleOrig = picture;
    }
    void setKetchupGameKetchupBottleCap(cv::Mat picture) {
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        ketchupBottleCapOrig = picture;
    }
    
    void setKetchupGameSingleFryWithoutKetchup(cv::Mat picture) {
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        ketchupGameSingleFryWithoutKetchupOrig = picture;
    }
    void setKetchupGameSingleFryWithKetchup(cv::Mat picture) {
        ketchupGameImagesInitialized = false;
        if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
        ketchupGameSingleFryWithKetchupOrig = picture;
    }
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */
    
    void initializeKetchupGameImages() {
        if (ketchupGameSceneryOrig.data != NULL) {
            ketchupGameScenery = ketchupGameSceneryOrig.clone();
        }
        if (plateWithFries1Orig.data != NULL) {
            plateWithFries = plateWithFries1Orig.clone();
        }
        if (plateWithFries2Orig.data != NULL) {
            //plateWithFries2 = plateWithFries2Orig.clone();
        }
        if (ketchupBottleCapOrig.data != NULL) {
            ketchupBottleCap = ketchupBottleCapOrig.clone();
        }
        if (ketchupBottleOrig.data != NULL) {
            ketchupBottle = ketchupBottleOrig.clone();
        }
        if (ketchupGameSingleFryWithoutKetchupOrig.data != NULL) {
            ketchupGameSingleFry = ketchupGameSingleFryWithoutKetchupOrig.clone();
        }
        if (ketchupGameSingleFryWithKetchupOrig.data != NULL) {
            //ketchupGameSingleFry = ketchupGameSingleFryWithKetchupOrig.clone();
        }
        
        ketchupGameImagesInitialized = true;
    }
    
    //maximum value before it breaks
    double get_max_ketchup_param() {
        return 1;
    }
    
    //minimum value before it will launch, also the vallue it starts to crack a bit
    double get_min_considering_ketchup_param() {
        return 0.3;
    }
    
    double calculate_score_increase_for_ketchup_game() {
        if (currentKetchupParam != -1) {
            return 100*(currentKetchupParam-KETCHUP_STARTING_PARAM)/(get_max_ketchup_param()-KETCHUP_STARTING_PARAM);
        } else {
            return 0;
        }
        
    }
    
    cv::Point rotatePtAroundPt(cv::Point ptToRotate, double radians, cv::Point aroundPt) {
        double x1 = ptToRotate.x;
        double y1 = ptToRotate.y;
        double x0 = aroundPt.x;
        double y0 = aroundPt.y;
        double x2 =   ((x1 - x0) * cos(radians)) + ((y1 - y0) * sin(radians)) + x0;
        double y2 = - ((x1 - x0) * sin(radians)) + ((y1 - y0) * cos(radians)) + y0;
        return cv::Point(x2, y2);
    }
    
    // main method that does game graphics and rotation calculation, returning angle
    double do_ketchup_game(Mat& fgmask, Mat &frame_with_mask, int dir, double ratio) {
        std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration time_span = nowTime - continuousGameResultStartTime;
        double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
        
        double param = 0.0;
        double gameResultCutoff = 2; //seconds
        
        
        if (!ketchupGameImagesInitialized) {
            initializeKetchupGameImages();
        }
        
        continuousGameAnimationState = time_diff > gameResultCutoff ? GAME_STATE_NORMAL_GAMEPLAY : GAME_STATE_CONTINUOUS_GAME_ANIMATION;
        bool isExplodedAnimation = continuousGameAnimationState==GAME_STATE_CONTINUOUS_GAME_ANIMATION && currentKetchupParam >= -1.0001 && currentKetchupParam < -0.9999;
        if (continuousGameAnimationState == GAME_STATE_NORMAL_GAMEPLAY) {// if normal gameplay, not ending game animation
            if (time_diff < gameResultCutoff*100) {
                //make continuousGameResultStartTime a really big number so this only happens once (beyond the *100 threshold)
                continuousGameResultStartTime = std::chrono::steady_clock::now() - std::chrono::hours(24);
                currentKetchupParam = KETCHUP_STARTING_PARAM;
                ketchupParamIncreaseAmt = 0;
                
                ketchupGameImagesInitialized = false;
                initializeKetchupGameImages();
            }
            if (currentKetchupParam == -1) {
                currentKetchupParam = KETCHUP_STARTING_PARAM;
                ketchupParamIncreaseAmt = 0;
            }
            
            //don't do analysis if fgmask is too small because small particles of noise have a drastic effect on a continuous game like ketchup game because of the signal processing
            if (cv::countNonZero(fgmask)*1.0/(fgmask.rows*fgmask.cols) > 0.1) {
                //do the actual analysis of the hand
                //false means do not add feedback arrows to the image
                param = analyze_pronation(fgmask, frame_with_mask, ratio); //(fgmask, frame_with_mask, dir, ratio, shouldAddFeedBackArrows = false);
                //cout << param << endl;
                if (param < 0 && !isParamNoExerciseDone(param)) param = -param; //absolute value (but keeps sign for appropriate special constants like PARAM_NOT_VALID and PARAM_NO_HAND)
            }
            
            
        } else {
            // -1000 signals to analyzeForCatapultGame that the animation is being done
            // analyzeForCatapultGame converts param to 0 before reporting it higher up though
            param = -1000;
        }
        
        
    
        
        if (ketchupGameSceneryOrig.data != NULL) {
            ///scenery must be rewritten every time because the ketchup bottle is written to the scenery
            if (true || ketchupGameScenery.data == NULL || ketchupGameScenery.rows != frame_with_mask.rows || ketchupGameScenery.cols != frame_with_mask.cols) {
                cv::resize(ketchupGameSceneryOrig, ketchupGameScenery, frame_with_mask.size(),0,0, INTER_NEAREST );
            }
            
            Scalar ketchupColor = ketchupGameScenery.channels() == 4 ? Scalar(164,33,45,255) : Scalar(164,33,45);

            int plateWithFriesHt = 0.3*frame_with_mask.rows;
            int plateWithFriesWd = plateWithFriesHt; //placeholder value
            int start_x_plate = 0.07*frame_with_mask.cols; //change it for left hand later
            int start_y_plate = 0.85*frame_with_mask.rows - plateWithFriesHt;
            
            double numCycles = 20;
            double amplitude = 3;
            double avgAngle = 35;
            
            int bottleHt = frame_with_mask.rows*0.45;
            int bottleWd = bottleHt; //assume square image because we are rotating it
            if (plateWithFries1Orig.data != NULL) {
                plateWithFriesWd = plateWithFriesHt*plateWithFries1Orig.cols*1.0/plateWithFries1Orig.rows;
                if (whichHand == Hand::LEFT) {
                    start_x_plate = frame_with_mask.cols-start_x_plate-plateWithFriesWd;
                }
            }
            int start_x_bottle = start_x_plate+plateWithFriesWd*(KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES)-bottleWd*(0.5-0.5*cos(M_PI/180*avgAngle));
            //int start_x_bottle = start_x_plate + plateWithFriesWd - frame_with_mask.cols*0.1;
            int start_y_bottle = start_y_plate - bottleHt; //the bottle image already has vertical spacing that is transparent
            if (whichHand == Hand::LEFT) {
                //start_x_bottle = start_x_plate+plateWithFriesWd*KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES;
                start_x_bottle = start_x_plate+plateWithFriesWd*(1-KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES)-bottleWd*(0.5+0.5*cos(M_PI/180*avgAngle));
                //plateWithFriesWd*KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES
            }
            //start_x_bottle = start_x_plate;
            
            cv::Point plateKetchupCenter = cv::Point(-1,-1); //placeholder value
            if (plateWithFries1Orig.data != NULL) {
                double scale = plateWithFriesHt*1.0/plateWithFries1Orig.rows;
                //if (plateWithFries.data == NULL || plateWithFries.rows != plateWithFriesHt || plateWithFries.cols != plateWithFriesWd) {
                //Do this every line otherwise old drawings of the ketchup bottle will be there
                cv::resize(plateWithFries1Orig, plateWithFries, cv::Size(), scale, scale, INTER_NEAREST );
                //}
                
                double center_x_ketchup = start_x_plate+plateWithFriesWd*KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES;
                if (whichHand == Hand::LEFT) {
                    center_x_ketchup = start_x_plate+plateWithFriesWd*(1-KETCHUP_CENTER_X_LOC_WITHIN_PLATEWITHFRIES);
                }
                plateKetchupCenter = cv::Point(center_x_ketchup,start_y_plate+plateWithFriesHt*0.5);
                
                
                if (ketchupGameSingleFryWithoutKetchupOrig.data != NULL) {
                    if (continuousGameAnimationState==GAME_STATE_CONTINUOUS_GAME_ANIMATION && !isExplodedAnimation && time_diff/(0.5*gameResultCutoff)>1.125) {
                        cv::resize(ketchupGameSingleFryWithKetchupOrig, ketchupGameSingleFry, cv::Size(), scale, scale, INTER_NEAREST );
                    } else {
                        cv::resize(ketchupGameSingleFryWithoutKetchupOrig, ketchupGameSingleFry, cv::Size(), scale, scale, INTER_NEAREST );
                    }
                    
                    
                    int top    = max(0,ketchupGameSingleFry.cols-ketchupGameSingleFry.rows)/2;
                    int bottom = max(0,ketchupGameSingleFry.cols-ketchupGameSingleFry.rows)-top;
                    int left   = max(0,ketchupGameSingleFry.rows-ketchupGameSingleFry.cols)/2;
                    int right  = max(0,ketchupGameSingleFry.rows-ketchupGameSingleFry.cols)-left;
                    cv::copyMakeBorder(ketchupGameSingleFry, ketchupGameSingleFry, top, bottom, left, right, BORDER_CONSTANT, Scalar(0,0,0,0)); //transparent

                    
                    int start_x_singlefry = start_x_plate;
                    if (whichHand == Hand::LEFT) {
                        start_x_singlefry = start_x_plate + plateWithFriesWd - ketchupGameSingleFry.cols;
                    }

                    int start_y_singlefry = start_y_plate + plateWithFriesHt*SINGLEFRY_CENTER_Y_LOC_WITHIN_PLATEWITHFRIES - ketchupGameSingleFry.rows/2;
                    if (continuousGameAnimationState==GAME_STATE_CONTINUOUS_GAME_ANIMATION && !isExplodedAnimation) {
                        double time_percent1 = time_diff/(0.5*gameResultCutoff); //time_percent1 ranges from 0 to 2
                        Point2f pivotPt(ketchupGameSingleFry.cols / 2., ketchupGameSingleFry.rows / 2.);
                        double degrees = -60*max(1.0, min(1.0,time_percent1) ); //in degrees, not radians. Positive values mean counterclockwise.
                        if (whichHand == Hand::LEFT) {
                            degrees = -degrees;
                        }
                        Mat rotMat = getRotationMatrix2D(pivotPt, degrees, 1.0);
                        warpAffine(ketchupGameSingleFry, ketchupGameSingleFry, rotMat,
                                   Size(ketchupBottle.cols, ketchupBottle.rows));
                        
                        double arcdiameter = abs( (start_x_singlefry+pivotPt.x) -plateKetchupCenter.x);
                        
                        //1 time unit of moving towards the ketchup in a circular motion
                        //0.25 time units of being around the ketchup (vertical movement is rested, slight horizontal movement)
                        //0.5 time units of moving away from the ketchup (straight vertical, sinusoidal x)
                        //0.25 time units complete rest
                        
                        //double delta_start_x_singlefry = arcdiameter*( 1 - cos( M_PI * min(1.0,time_percent1)) ) / 2;
                        double percent_x_param = time_percent1;
                        if (percent_x_param > 1) percent_x_param = (percent_x_param-1)/0.75+1;
                        double delta_start_x_singlefry = arcdiameter*( 1 - cos( M_PI * min(1.75,percent_x_param)) ) / 2;
                        if (whichHand == Hand::LEFT) {
                            delta_start_x_singlefry = -delta_start_x_singlefry;
                        }
                        start_x_singlefry += delta_start_x_singlefry;
                        
                        if (time_percent1 < 1) { //time_percent1 ranges from 0 to 2
                            start_y_singlefry -=  arcdiameter*( sin( M_PI * min(1.0,time_percent1)) ) / 2;
                        } else if (time_percent1 >= 1.25) {
                            start_y_singlefry -= ( min(time_percent1,1.75) - 1.25)/0.5*start_y_singlefry;
                        }
                    }
                    
                    drawImageOptimized(ketchupGameScenery, plateWithFries, start_x_plate, start_y_plate);
                    
                    drawImageOptimized(ketchupGameScenery, ketchupGameSingleFry, start_x_singlefry, start_y_singlefry);

                    
                    double percent;
                    if (isExplodedAnimation) {
                        percent = (time_diff/gameResultCutoff*KETCHUP_PILE_EXTRA_RADIUS_EXPLODE_RATIO + 1)*KETCHUP_PILE_RADIUS_TO_PLATE_RATIO;
                    } else {
                        percent = currentKetchupParam/get_max_ketchup_param() * KETCHUP_PILE_RADIUS_TO_PLATE_RATIO;
                    }
                    cv::Size axes = cv::Size( plateWithFriesWd*percent, plateWithFriesHt*percent);
                    cv::ellipse(ketchupGameScenery, plateKetchupCenter, axes, 30, 0, 360, ketchupColor, FILLED);
                }
                
                //cout << currentKetchupParam << " " << param << endl;
            }
            

            
            

            Mat bgmask;
            bitwise_not(fgmask, bgmask);
            cv::resize(bgmask, bgmask, frame_with_mask.size(), 0,0, INTER_NEAREST );
            bitwise_and(ketchupGameScenery, ketchupGameScenery, frame_with_mask, bgmask);
            
            
            if (ketchupBottleOrig.data != NULL) {
                if (ketchupBottle.data == NULL || ketchupBottle.rows != bottleHt || ketchupBottle.cols != bottleWd) {
                    cv::resize(ketchupBottleOrig, ketchupBottle, cv::Size(bottleWd,bottleHt), 0,0, INTER_NEAREST );
                }
                
                Mat ketchupBottleWithCap = ketchupBottle.clone();
                int ketchupBottleEndXLoc = CAP_RIGHT_X_LOC_WITHIN_KETCHUP_BOTTLE*ketchupBottleWithCap.cols; //this is the point where the bottle ends and space to the left is space meant for the cap
                if (ketchupBottleCapOrig.data != NULL && !isExplodedAnimation) {
                    //assume that the ketchupBottleCapOrig and ketchupBottleOrig images are proportional sizes when inputted in
                    int bottleCapHt = bottleHt * ketchupBottleCapOrig.rows*1.0/ketchupBottleOrig.rows;
                    int bottleCapWd = bottleWd * ketchupBottleCapOrig.cols*1.0/ketchupBottleOrig.cols;
                    if (ketchupBottleCap.data == NULL || ketchupBottleCap.rows != bottleCapHt || ketchupBottleCap.cols != bottleCapWd) {
                        cv::resize(ketchupBottleCapOrig, ketchupBottleCap, cv::Size(bottleCapWd,bottleCapHt), 0,0, INTER_NEAREST );
                    }
                    int start_x_bottlecap =  ketchupBottleEndXLoc - ketchupBottleCap.cols;
                    if (whichHand == Hand::LEFT) {
                        start_x_bottlecap =  ketchupBottleWithCap.cols - ketchupBottleEndXLoc;
                    }
                    int start_y_bottlecap = (ketchupBottle.rows-ketchupBottleCap.rows)/2;
                    drawImageOptimized(ketchupBottleWithCap, ketchupBottleCap, start_x_bottlecap, start_y_bottlecap );
                }
                
                Point2f pivotPtBottle(ketchupBottle.cols / 2., ketchupBottle.rows / 2.);
                double degrees = avgAngle+amplitude*sin(currentKetchupParam/get_max_ketchup_param()*numCycles*2*M_PI); //in degrees, not radians. Positive values mean counterclockwise.
                if (whichHand == Hand::LEFT) {
                    degrees = -degrees;
                }
                //cout << degrees << endl;

                Mat rotMat = getRotationMatrix2D(pivotPtBottle, degrees, 1.0);
                warpAffine(ketchupBottleWithCap, ketchupBottleWithCap, rotMat,
                           Size(ketchupBottle.cols, ketchupBottle.rows));
                
                //only have the ketchup flowing if normally game play or exploding animation
                if (isExplodedAnimation || continuousGameAnimationState==GAME_STATE_NORMAL_GAMEPLAY) {
                    int measurement_size = 10;
                    double sumDiff = 0;
                    if (paramHistory.size() >= measurement_size+1) {
                        double laterParam = paramHistory[paramHistory.size()-1];
                        double priorParam;
                        for (long i = paramHistory.size()-1-1; i>=paramHistory.size()-measurement_size; i--) {
                            priorParam = paramHistory[i];
                            if (isParamNoHand(priorParam) || isParamNoHand(laterParam)) {
                                
                            } else if (isParamNoExerciseDone(priorParam) || isParamNoExerciseDone(laterParam)) {
                                goto drawFlow;
                                
                            } else {
                                sumDiff = sumDiff + abs(priorParam-laterParam);
                                
                            }
                            laterParam = priorParam;
                        }
                    }
                    
                    drawFlow:
                    //int ketchupFlowThickness = isExplodedAnimation ? 10 : std::min(abs(ketchupParamIncreaseAmt)/0.0001,5.0);
                    int ketchupFlowThickness = isExplodedAnimation ? 10 : std::min(sumDiff/0.3, 5.0);
                    
                    cv::Point aroundPt = cv::Point2f(start_x_bottle, start_y_bottle) + pivotPtBottle;
                    //startFlowIfStraight is the center point of the cap if degrees were 0
                    cv::Point startFlowIfStraight;
                    if (whichHand == Hand::LEFT) {
                        startFlowIfStraight = cv::Point(start_x_bottle+ketchupBottleWithCap.cols-ketchupBottleEndXLoc/2, start_y_bottle+ketchupBottleWithCap.rows/2);
                    } else {
                        startFlowIfStraight = cv::Point(start_x_bottle+ketchupBottleEndXLoc/2, start_y_bottle+ketchupBottleWithCap.rows/2);
                    }
                    cv::Point startFlow = rotatePtAroundPt(startFlowIfStraight, degrees*M_PI/180, aroundPt);
                    cv::Point endFlow = cv::Point(startFlow.x, plateKetchupCenter.y);
                    
                    if (ketchupFlowThickness > 0) {
                        cv::line(frame_with_mask, startFlow, endFlow, ketchupColor, ketchupFlowThickness);
                        if (isExplodedAnimation) { //otherwise, this is not necessary because hidden by the cap
                            cv::line(frame_with_mask, aroundPt, startFlow, ketchupColor, ketchupFlowThickness);
                        }
                    }
                    
                    
                    drawImageOptimized(frame_with_mask, ketchupBottleWithCap, start_x_bottle, start_y_bottle);
                    
                    if (isExplodedAnimation) {
                        double start_x_cap = startFlow.x - ketchupBottleCap.cols; //convert from startFlow which is center point to the left point
                        double start_y_cap = startFlow.y - ketchupBottleCap.rows; //convert from startFlow which is center point to the top point
                        start_y_cap += time_diff/gameResultCutoff*(frame_with_mask.rows-startFlow.y);
                        drawImageOptimized(frame_with_mask, ketchupBottleCap, start_x_cap, start_y_cap);
                    }
                } else {
                    drawImageOptimized(frame_with_mask, ketchupBottleWithCap, start_x_bottle, start_y_bottle);

                }
                
            }
        }
        
            
        
        
        
        return param;
    }
    
#ifdef EXTERN_C
}
#endif
