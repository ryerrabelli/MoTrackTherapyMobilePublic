//
// Created by Rahul Yerrabelli on 02/21/19..
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

#include "deviation_windshield.h"
#include "deviation_catapult.h"
#include "bilinear_approx.h"



using namespace cv;
using namespace std;


#ifdef EXTERN_C
extern "C" {
#endif
    
    //const variables that hold coordinates are measured as percentage of frame dimensions (height if dimension-neutral)
    const double BULLSEYE_DIAMETER = 0.25; //strictly measured as the height of bullseye image
    const double BULLSEYE_BOTTOM_Y_LOC = (231+276)/2.0/540;
    const double BULLSEYE_CENTER_X_LOC = 122.5/720;
    const double CATAPULT_MOUNT_HEIGHT_RATIO = 0.3;
    const double CATAPULT_BEAM_CENTER_X_LOC_ON_MOUNT = 260.0/856; //location of pivot point relative to the catapult mount
    const double CATAPULT_BEAM_CENTER_Y_LOC_ON_MOUNT = 340.0/422;
    const double PAYLOAD_DIAMETER = 110.0/1148; //radius as a percentage of the width of the catapult beam image (which is about twice the catapult beam because of all the whitespace needed to make a good pivot).
    const double PAYLOAD_ARC_RADIUS = 509.0/1148; //denotes where along the beam the payload should be (along as in the long way along)
    const double PAYLOAD_RADIUS_SIZE_DECREASE_BY_DISTANCE = 2.0/3.0; //payloadRadius becomes 1/3 of its size by the time it reaches the catapult
    
    const double CATAPULT_STARTING_PARAM = 0.0;
    double currentCatapultParam = CATAPULT_STARTING_PARAM;
    cv::Mat bullseye;
    cv::Mat catapultMount; //(renamed from catapult frame because of confusion)
    cv::Mat catapultBeam;
    cv::Mat fieldScenery;
    
    cv::Mat catapultMountBrokenFront;
    cv::Mat catapultMountBrokenBack;
    cv::Mat catapultMountCracked;
    
    cv::Mat bullseyeOrig;
    cv::Mat catapultMountOrig;
    cv::Mat catapultBeamOrig;
    cv::Mat fieldSceneryOrig;
    
    cv::Mat catapultMountBrokenFrontOrig;
    cv::Mat catapultMountBrokenBackOrig;
    cv::Mat catapultMountCrackedOrig;
    
    bool catapultGameImagesInitialized = false;
    

#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setFieldScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            //below line would set it directly. Can't do this because we want to convert the color
            //bullseye = *matAddress;
            cv::cvtColor(*matAddress, fieldSceneryOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }

    void
    Java_com_motracktherapy_motrack_CVActivity_setBullseye(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            //below line would set it directly. Can't do this because we want to convert the color
            //bullseye = *matAddress;
            cv::cvtColor(*matAddress, bullseyeOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }

    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultBeam(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            //below line would set it directly. Can't do this because we want to convert the color
            //bullseye = *matAddress;
            cv::cvtColor(*matAddress, catapultBeamOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }


    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultMount(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            //below line would set it directly. Can't do this because we want to convert the color
            //bullseye = *matAddress;
            cv::cvtColor(*matAddress, catapultMountOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }

    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultMountCracked(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::cvtColor(*matAddress, catapultMountCrackedOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }

    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultMountBrokenFront(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::cvtColor(*matAddress, catapultMountBrokenFrontOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }

    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultMountBrokenBack(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (addr != -1) {
            cv::Mat *matAddress = (cv::Mat *) addr;
            cv::cvtColor(*matAddress, catapultMountBrokenBackOrig, cv::COLOR_BGRA2RGBA);
            catapultGameImagesInitialized = false;
        }
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
void setFieldScenery(cv::Mat picture) {
    if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
    catapultGameImagesInitialized = false;
    fieldSceneryOrig = picture;
}
void setBullseye(cv::Mat picture) {
    catapultGameImagesInitialized = false;
    bullseyeOrig = picture;
}
void setCatapultBeam(cv::Mat picture) {
    catapultGameImagesInitialized = false;
    catapultBeamOrig = picture;
}
void setCatapultMount(cv::Mat picture) {
    catapultGameImagesInitialized = false;
    catapultMountOrig = picture;
}
void setCatapultMountBrokenFront(cv::Mat picture) {
    catapultGameImagesInitialized = false;
    catapultMountBrokenFrontOrig = picture;
}
void setCatapultMountBrokenBack(cv::Mat picture) {
    catapultGameImagesInitialized = false;
    catapultMountBrokenBackOrig = picture;
}
void setCatapultMountCracked(cv::Mat picture) {
    catapultGameImagesInitialized = false;
    catapultMountCrackedOrig = picture;
}

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */

void initializeCatapultGameImages() {
    if (fieldSceneryOrig.data != NULL) {
        if (whichHand == Hand::LEFT) {
            cv::flip(fieldSceneryOrig, fieldSceneryOrig, +1);
        }
        fieldScenery = fieldSceneryOrig.clone();
    }
    if (bullseyeOrig.data != NULL) {
        bullseye = bullseyeOrig.clone();
        if (whichHand == Hand::LEFT) {
            cv::flip(bullseyeOrig, bullseyeOrig, +1);
        }
        bullseye = bullseyeOrig.clone();
    }
    if (catapultBeamOrig.data != NULL) {
        if (whichHand == Hand::LEFT) {
            cv::flip(catapultBeamOrig, catapultBeamOrig, +1);
        }
        catapultBeam = catapultBeamOrig.clone();
    }
    if (catapultMountOrig.data != NULL) {
        if (whichHand == Hand::LEFT) {
            cv::flip(catapultMountOrig, catapultMountOrig, +1);
        }
        catapultMount = catapultMountOrig.clone();
    }
    if (catapultMountBrokenFrontOrig.data != NULL) {
        if (whichHand == Hand::LEFT) {
            cv::flip(catapultMountBrokenFrontOrig, catapultMountBrokenFrontOrig, +1);
        }
        catapultMountBrokenFront = catapultMountBrokenFrontOrig.clone();
    }
    if (catapultMountBrokenBackOrig.data != NULL) {
        if (whichHand == Hand::LEFT) {
            cv::flip(catapultMountBrokenBackOrig, catapultMountBrokenBackOrig, +1);
        }
        catapultMountBrokenBack = catapultMountBrokenBackOrig.clone();
    }
    if (catapultMountCrackedOrig.data != NULL) {
        if (whichHand == Hand::LEFT) {
            cv::flip(catapultMountCrackedOrig, catapultMountCrackedOrig, +1);
        }
        catapultMountCracked = catapultMountCrackedOrig.clone();
    }

    catapultGameImagesInitialized = true;
}

//maximum value before it breaks
double get_max_catapult_param() {
    return 75; //units of degrees
}
    
//minimum value before it will launch, also the vallue it starts to crack a bit
double get_min_considering_catapult_param() {
    return 30; //units of degrees
}

double calculate_score_increase_for_catapult_game() {
    if (currentCatapultParam != -1) {
        return (currentCatapultParam-CATAPULT_STARTING_PARAM)/3.0*4;
    } else {
        return 0;
    }
    
}
    
// main method that does game graphics and bilinear approximation, returning angle
int do_catapult_game(Mat& fgmask, Mat &frame_with_mask, int dir, double ratio) {
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration time_span = nowTime - continuousGameResultStartTime;
    double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;

    int param = 0.0;
    double gameResultCutoffPt1 = 1.5; //seconds, //represents the catapult lunging forward for the launch
    double gameResultCutoffPt2 = 1.5; //seconds//represents ball in the air part of the launch
    double gameResultCutoff = gameResultCutoffPt1 + gameResultCutoffPt2; //seconds

    
    if (!catapultGameImagesInitialized) {
        initializeCatapultGameImages();
    }
    
    double drawBullseyeBallEnding = -1;
    
    continuousGameAnimationState = time_diff > gameResultCutoff ? GAME_STATE_NORMAL_GAMEPLAY : GAME_STATE_CONTINUOUS_GAME_ANIMATION;
    if (continuousGameAnimationState == GAME_STATE_NORMAL_GAMEPLAY) {// if normal gameplay, not ending game animation
        if (time_diff < gameResultCutoff*100) {
            drawBullseyeBallEnding = currentCatapultParam;
            //make continuousGameResultStartTime a really big number so this only happens once (beyond the *100 threshold)
            continuousGameResultStartTime = std::chrono::steady_clock::now() - std::chrono::hours(24);
            currentCatapultParam = CATAPULT_STARTING_PARAM;
        }
        if (currentCatapultParam == -1) {
            currentCatapultParam = CATAPULT_STARTING_PARAM;
        }
        
        //do the actual analysis of the hand
        //false means do not add feedback arrows to the image
        param = bilinear_approx_analysis(fgmask, frame_with_mask, dir, ratio, false); //approx returns an int
        
    } else {
        // -1000 signals to analyzeForCatapultGame that the animation is being done
        // analyzeForCatapultGame converts param to 0 before reporting it higher up though
        param = -1000;
    }
    
    

    int bullseye_center_x = 0;
    int bullseye_center_y = 0;
    
    Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(textWriteRectangle.width, 0, frame_with_mask.cols-textWriteRectangle.width, frame_with_mask.rows)) : frame_with_mask;
    int bullseye_diameter = BULLSEYE_DIAMETER * frame_with_mask_draw.rows; //diameter technically measured as the "height"
    
    if (fieldSceneryOrig.data != NULL) {
        cv::resize(fieldSceneryOrig, fieldScenery, cv::Size(frame_with_mask_draw.cols, frame_with_mask_draw.rows), 0, 0, INTER_LINEAR);
        
        //Draw Bullseye
        if (bullseyeOrig.data != NULL) {
            if ( abs(bullseye.rows*1.0/frame_with_mask_draw.rows - BULLSEYE_DIAMETER) > 0.01 ) {
                cv::resize(bullseyeOrig, bullseye, cv::Size(BULLSEYE_DIAMETER * frame_with_mask_draw.rows, BULLSEYE_DIAMETER * frame_with_mask_draw.rows) );
            } else {
                bullseye_diameter = bullseye.rows;
            }

            int start_x = frame_with_mask_draw.cols*BULLSEYE_CENTER_X_LOC - bullseye.cols/2; //convert from from center_x_loc position to left side
            int start_y = frame_with_mask_draw.rows*BULLSEYE_BOTTOM_Y_LOC - bullseye.rows;   //convert from from bottom_y_loc position to top side
            // start x = 154
            if (whichHand == Hand::LEFT) { // left
                start_x = frame_with_mask_draw.cols - start_x-bullseye.cols;
            }
            drawImageOptimized(fieldScenery, bullseye, start_x, start_y);
            
            bullseye_center_x = start_x + bullseye.cols/2;
            bullseye_center_y = start_y + bullseye.rows/2;
        }
        
        // for text that will be written on top/side
        if (shouldWriteOnIm) {
            //Don't need to draw the rectangle since copyMakeBorder already does the same effect
            cv::copyMakeBorder(fieldScenery, fieldScenery, 0, 0, textWriteRectangle.width, 0, BORDER_CONSTANT, textWriteRectangleBackground);
            //cv::rectangle(fieldScenery, textWriteRectangle, textWriteRectangleBackground, -1, 8);

            cv::line(fieldScenery, cv::Point(textWriteRectangle.width,0), cv::Point(textWriteRectangle.width, fieldScenery.rows), CLR_BLACK_4UC, 5);

        }
        
        Mat bgmask;
        bitwise_not(fgmask, bgmask);
        cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
        bitwise_and(fieldScenery, fieldScenery, frame_with_mask, bgmask);
        
    }
    
    
    
    //Draw catapult mount
    if (catapultMountOrig.data != NULL) {
        int catapult_mount_ht = CATAPULT_MOUNT_HEIGHT_RATIO * frame_with_mask_draw.rows;
        if ( abs(catapultMount.rows*1.0/frame_with_mask_draw.rows - CATAPULT_MOUNT_HEIGHT_RATIO) > 0.01 || true) {
            double scale = catapult_mount_ht*1.0/catapultMountOrig.rows; // > 1 if need to expand
            cv::resize(catapultMountOrig, catapultMount, cv::Size(), scale, scale  );
            if (catapultMountCrackedOrig.data != NULL) {
                cv::resize(catapultMountCrackedOrig, catapultMountCracked, cv::Size(), scale, scale  );
            }
            if (catapultMountBrokenFrontOrig.data != NULL) {
                cv::resize(catapultMountBrokenFrontOrig, catapultMountBrokenFront, cv::Size(), scale, scale  );
            }
            if (catapultMountBrokenBackOrig.data != NULL) {
                cv::resize(catapultMountBrokenBackOrig, catapultMountBrokenBack, cv::Size(), scale, scale  );
            }
            if (catapultBeamOrig.data != NULL) {
                cv::resize(catapultBeamOrig, catapultBeam, cv::Size(), scale, scale);
            }
        } else {
            catapult_mount_ht = catapultMount.rows;
        }
        int catapult_mount_wd = catapultMount.cols;
    
        //must subtract by -1 or drawImageOptimized will for some reason make catapultMount smaller every frame it is called (maybe due to a bug?)
        int start_x_mount = frame_with_mask_draw.cols - catapultMount.cols - 1;
        int start_y_mount = int(double(frame_with_mask_draw.rows - catapultMount.rows)* 0.9);
        // start_x_mount = 841
        if (whichHand == Hand::LEFT) { // left
            start_x_mount = 1;//frame_with_mask.cols - start_x_mount;
        }
        
        //Draw catapult beam (must be done before the frame, so that it is below it)
        if (catapultBeamOrig.data != NULL && currentCatapultParam > -0.5) { //ignore if currentCatapultParam is -1
            float center_x_beam;
            if (whichHand == Hand::LEFT) {
                center_x_beam = start_x_mount + (1-CATAPULT_BEAM_CENTER_X_LOC_ON_MOUNT)*catapult_mount_wd - 1;
            } else {
                center_x_beam = start_x_mount + CATAPULT_BEAM_CENTER_X_LOC_ON_MOUNT*catapult_mount_wd - 1;
            }
            float center_y_beam = start_y_mount + CATAPULT_BEAM_CENTER_Y_LOC_ON_MOUNT*catapult_mount_ht - 1;

            int start_x_beam = center_x_beam - catapultBeam.cols/2.0;
            int start_y_beam = center_y_beam - catapultBeam.rows/2.0;

            double rotateBeamAmt = 0;
            double rotatePayloadAmt = -5;
            if (time_diff > gameResultCutoff) { //normal game running
                rotateBeamAmt = currentCatapultParam;
                rotatePayloadAmt = rotateBeamAmt - 5; //subtract by 5 so that currentCatapultParam is a little bit (5 degrees) in front of the beam
                
            } else if (currentCatapultParam == -1) { //catapult broke
                
                
            } else {
                if (time_diff < gameResultCutoffPt1) { //catapult launching animation
                    double time_percent = (time_diff)/gameResultCutoffPt1;
                    rotateBeamAmt = currentCatapultParam*(1-time_percent); //decrease angle linearly with time
                    rotatePayloadAmt = rotateBeamAmt - 5;
                    
                } else { //catapult launched and in the air
                    rotateBeamAmt = 0;
                    rotatePayloadAmt = rotateBeamAmt - 5;
                    
                }
                
            }
            
            Mat catapultBeamRotated = catapultBeam.clone();
            Point2f pt(catapultBeamRotated.cols / 2., catapultBeamRotated.rows / 2.);
            double degrees = 90-rotateBeamAmt; //in degrees, not radians. Positive values mean counterclockwise.
            if (whichHand == Hand::LEFT) { // left
                degrees = -degrees;
            }
            Mat rotMat = getRotationMatrix2D(pt, degrees, 1.0);
            warpAffine(catapultBeamRotated, catapultBeamRotated, rotMat,
                    Size(catapultBeamRotated.cols, catapultBeamRotated.rows));
            
            //Draw payload (need to do before drawing beam so that the payload is behind the beam)
            //cout << currentCatapultParam << endl;
            int payloadRadius = PAYLOAD_DIAMETER/2*catapultBeam.cols;
            double payloadArcRadius = PAYLOAD_ARC_RADIUS * catapultBeam.cols;
            Point payloadCenter;
            payloadCenter.x = start_x_beam + catapultBeam.cols/2.0 * 1 + payloadArcRadius*sin( rotatePayloadAmt*3.1415/180);
            payloadCenter.y = start_y_beam + catapultBeam.rows/2.0 * 1 - payloadArcRadius*cos( rotatePayloadAmt*3.1415/180);

            if (whichHand == Hand::LEFT) { // left
                payloadCenter.x = start_x_beam + catapultBeam.cols/2.0 * 1 - payloadArcRadius*sin( rotatePayloadAmt*3.1415/180);
            }

            if ( ((time_diff <= gameResultCutoff && time_diff > gameResultCutoffPt1)) ||  drawBullseyeBallEnding > 0) {
                //double bullseye_angle = 45 * M_PI/180; //what angle on the bullseye should the payload land

                if (time_diff <= gameResultCutoff && time_diff > gameResultCutoffPt1) { //catapult launched and in the air
                    //get a bullseye angle between -135 and +45 so it faces the catapult (for right hand, and subtract -90 degrees or pi/2 radians for left)
                    double bullseye_angle = (fmod(currentCatapultParam*1000,180)-135) * M_PI/180; //what angle on the bullseye should the payload land
                    if (whichHand == Hand::LEFT) bullseye_angle -= M_PI/2; //flip sides
                    double accuracyPercent = (1-currentCatapultParam/get_max_catapult_param());
                    int final_x = bullseye_center_x + bullseye_diameter*accuracyPercent*cos(bullseye_angle);
                    int final_y = bullseye_center_y - bullseye_diameter*accuracyPercent*sin(bullseye_angle); //negative sign because positive y is downward
                    double time_percent2 = (time_diff-gameResultCutoffPt1)/gameResultCutoffPt2;
                    payloadCenter.x = payloadCenter.x + (final_x-payloadCenter.x)*time_percent2;
                    /*if (whichHand == Hand::LEFT) { // left
                     payloadCenter.x = frame_with_mask.cols - payloadCenter.x;
                     }*/
                    payloadCenter.y = payloadCenter.y + (final_y-payloadCenter.y)*time_percent2;
                    payloadRadius = payloadRadius * (1-time_percent2*PAYLOAD_RADIUS_SIZE_DECREASE_BY_DISTANCE); //payloadRadius becomes a fraction of its size by the time it reaches the catapult
                    
                } else if (drawBullseyeBallEnding > 0) {//draw the ball final state from last time
                    //get a bullseye angle between -135 and +45 so it faces the catapult (for right hand, and subtract -90 degrees or pi/2 radians for left)
                    double bullseye_angle = (fmod(drawBullseyeBallEnding*1000,180)-135) * M_PI/180; //what angle on the bullseye should the payload land
                    if (whichHand == Hand::LEFT) bullseye_angle -= M_PI/2; //flip sides
                    double accuracyPercent = (1-drawBullseyeBallEnding/get_max_catapult_param());
                    int final_x = bullseye.cols/2.0 + bullseye_diameter*accuracyPercent*cos(bullseye_angle);
                    int final_y = bullseye.rows/2.0 - bullseye_diameter*accuracyPercent*sin(bullseye_angle); //negative sign because positive y is downward
                    cv::circle(bullseye, cv::Point(final_x,final_y), payloadRadius*(1-PAYLOAD_RADIUS_SIZE_DECREASE_BY_DISTANCE), CLR_BLACK_4UC, 1+payloadRadius*0.05); //Draw border (add 1 to round up, not down)
                    cv::circle(bullseye, cv::Point(final_x,final_y), payloadRadius*0.95*(1-PAYLOAD_RADIUS_SIZE_DECREASE_BY_DISTANCE), CLR_MOTRACK_BLUE_4UC, cv::FILLED); //Draw center
                    
                }
            }
            
            cv::circle(frame_with_mask_draw, payloadCenter, payloadRadius, CLR_BLACK_4UC, 1+payloadRadius*0.05); //Draw border (add 1 to round up, not down)
            cv::circle(frame_with_mask_draw, payloadCenter, payloadRadius*0.95, CLR_MOTRACK_BLUE_4UC, cv::FILLED); //Draw center
            
            
            drawImageOptimized(frame_with_mask_draw, catapultBeamRotated, start_x_beam, start_y_beam);

        }
        
        //Draw the catapult mount (will draw differently, depending on situation)
        if (currentCatapultParam == -1) { //catapult broke
            //Note: We rotate the broken parts. These images are not square images and this is the first time we do it on a non-square image. Doing it on a non-square image means that the image can get cutoff in rotation. We kept them as non-square images because we figured it would be better if the catapultMount images lined up. Plus, the images won't be cutoff if we rotate them towards from their respective whitespace-heavy sides.
            double time_percent = (time_diff)/gameResultCutoff;
            //make time_percent constant for the second half of the time, so there is some rest pe
            time_percent = std::min(1.0,time_percent*2);
            double degrees = 30*std::min(1.0,time_percent*2); //in degrees, not radians. Positive values mean counterclockwise.
            if (whichHand == Hand::LEFT) { // left
                degrees = -degrees;
            }
            
            if (catapultMountBrokenFrontOrig.data != NULL) {
                Mat catapultMountBrokenFrontRotated = catapultMountBrokenFront.clone();
                Point2f ptFront(catapultMountBrokenFrontRotated.cols / 2., catapultMountBrokenFrontRotated.rows / 2.);
                Mat rotMatFront = getRotationMatrix2D(ptFront, degrees, 1.0);
                warpAffine(catapultMountBrokenFrontRotated, catapultMountBrokenFrontRotated, rotMatFront, Size(catapultMountBrokenFrontRotated.cols, catapultMountBrokenFrontRotated.rows));
                drawImageOptimized(frame_with_mask_draw, catapultMountBrokenFrontRotated, start_x_mount, start_y_mount);
            }
            
            if (catapultMountBrokenBackOrig.data != NULL) {
                Mat catapultMountBrokenBackRotated = catapultMountBrokenBack.clone();
                Point2f ptBack (catapultMountBrokenBackRotated.cols / 2.,  catapultMountBrokenBackRotated.rows / 2.);
                Mat rotMatBack = getRotationMatrix2D(ptBack,  -degrees, 1.0);
                warpAffine(catapultMountBrokenBackRotated,  catapultMountBrokenBackRotated,  rotMatBack,  Size(catapultMountBrokenBackRotated.cols,  catapultMountBrokenBackRotated.rows));
                drawImageOptimized(frame_with_mask_draw, catapultMountBrokenBackRotated, start_x_mount, start_y_mount);
            }
            
            
        } else if (currentCatapultParam < get_min_considering_catapult_param()) { //draw an uncracked amount because we just started the game and the catapult is completely healthy
            drawImageOptimized(frame_with_mask_draw, catapultMount, start_x_mount, start_y_mount);
            
        } else { //either normal gameplay or during launch
            //Note: Cracked != broken. cracked is just a visual cue that it will break somewhat soon
            if (catapultMountCrackedOrig.data != NULL) {
                drawImageOptimized(frame_with_mask_draw, catapultMountCracked, start_x_mount, start_y_mount);
            }
        }
       
    }
    
    
    return param;
}
    
#ifdef EXTERN_C
}
#endif
