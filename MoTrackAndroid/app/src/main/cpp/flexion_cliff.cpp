//
// Created by Rahul Yerrabelli on 3/6/19.
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

#include "flexion_cliff.h"
#include "bilinear_approx.h"



using namespace cv;
using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif
    const double CLIFF_WIDTH_RATIO = 1.0/3.0;
    const double CLIFF_STARTING_PARAM = 0.0;
    const double MAX_NUGGET_SIZE = 0.75; //percent relative to frame height
    const double CRACK_AREA_TOP_BORDER = 195.0/1125; //percent relative to scenery height
    const double CRACK_AREA_LEFT_BORDER= 0/780; //percent relative to scenery width
    const double CRACK_AREA_HEIGHT = 1085.0/1125; //percent relative to scenery height
    const double CRACK_AREA_WIDTH = 639.0/780; //percent relative to scenery width
    double currentCliffParam = CLIFF_STARTING_PARAM;
    cv::Mat cliffGameSceneryOrig;
    cv::Mat cliffGameScenery;
    cv::Mat cliffOrig;
    cv::Mat cliff;
    cv::Mat goldNuggetOrig;
    cv::Mat goldNugget;
    
    int catapultGameCracksDrawn = 0; //necessary to have this as a different variable than currentCliffParam so that we don't have to draw all the old lines each frame
    
    bool cliffGameImagesInitialized = false;
    
    
#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    void
    Java_com_motracktherapy_motrack_CVActivity_setCliffGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, cliffGameSceneryOrig, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(cliffGameSceneryOrig, cliffGameSceneryOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCliffGameCliff(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, cliffOrig, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(cliffOrig, cliffOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }

    void
    Java_com_motracktherapy_motrack_CVActivity_setCliffGameGoldNugget(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, goldNuggetOrig, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(goldNuggetOrig, goldNuggetOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }

    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * -------------------------------------------------------------------------------------------------
     *                                     iOS INTERFACE FUNCTIONS
     * -------------------------------------------------------------------------------------------------
     */
    
    void setCliffGameScenery(cv::Mat picture) {
        if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
        cliffGameSceneryOrig = picture;
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(cliffGameSceneryOrig, cliffGameSceneryOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }
    
    void setCliffGameCliff(cv::Mat picture) {
        cliffOrig = picture;
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(cliffOrig, cliffOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }
    
    void setCliffGameGoldNugget(cv::Mat picture) {
        goldNuggetOrig = picture;
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(goldNuggetOrig, goldNuggetOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */
    
    void initializeCliffGameImages(int width, int height) {
        if (cliffGameSceneryOrig.data != NULL) {
            cv::resize(cliffGameSceneryOrig, cliffGameScenery, cv::Size(width, height), 0, 0, INTER_LINEAR);
        }
        if (cliffOrig.data != NULL) {
            cv::resize(cliffOrig, cliff, cv::Size(cliffGameScenery.cols*CLIFF_WIDTH_RATIO, cliffGameScenery.cols));
        }
        
        cliffGameImagesInitialized = true;
    }
    
    //maximum value before it breaks
    double get_max_cliff_param() {
        return 75; //units of degrees
    }
    
    //minimum value before it will launch, also the vallue it starts to crack a bit
    double get_min_considering_cliff_param() {
        return 30; //units of degrees
    }
    
    double calculate_score_increase_for_cliff_game() {
        if (currentCliffParam != -1) {
            return (currentCliffParam-CLIFF_STARTING_PARAM)/3.0*4;
        } else {
            return 0;
        }
        
    }
    
    
    int do_cliff_game(Mat& fgmask, Mat &frame_with_mask, int dir, double ratio) {
        std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration time_span = nowTime - continuousGameResultStartTime;
        double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
        
        int param = 0.0;
        double gameResultCutoffPt1 = 1.5; //seconds, //represents the catapult lunging forward for the launch
        double gameResultCutoffPt2 = 1.5; //seconds//represents ball in the air part of the launch
        double gameResultCutoff = gameResultCutoffPt1 + gameResultCutoffPt2; //seconds
        
        if (!cliffGameImagesInitialized) {
            initializeCliffGameImages(frame_with_mask.cols, frame_with_mask.rows);
        }
        
        continuousGameAnimationState = time_diff > gameResultCutoff ? GAME_STATE_NORMAL_GAMEPLAY : GAME_STATE_CONTINUOUS_GAME_ANIMATION;
        if (continuousGameAnimationState == GAME_STATE_NORMAL_GAMEPLAY) {// if normal gameplay, not ending game animation
            if (time_diff < gameResultCutoff*100) {
                //make continuousGameResultStartTime a really big number so this only happens once (beyond the *100 threshold)
                continuousGameResultStartTime = std::chrono::steady_clock::now() - std::chrono::hours(24);
                currentCliffParam = CLIFF_STARTING_PARAM;
                catapultGameCracksDrawn = 0;
                cv::resize(cliffOrig, cliff, cv::Size(cliffGameScenery.cols*CLIFF_WIDTH_RATIO, cliffGameScenery.cols));
            }
            if (currentCliffParam == -1) {
                currentCliffParam = CLIFF_STARTING_PARAM;
                catapultGameCracksDrawn = 0;
                cv::resize(cliffOrig, cliff, cv::Size(cliffGameScenery.cols*CLIFF_WIDTH_RATIO, cliffGameScenery.cols));
            }
            
            //do the actual analysis of the hand
            //false means do not add feedback arrows to the image
            param = bilinear_approx_analysis(fgmask, frame_with_mask, dir, ratio, false); //approx returns an int
            
        } else {
            // -1000 signals to analyzeForCatapultGAme that the animation is being done
            // analyzeForCatapultGame converts param to 0 before reporting it higher up though
            param = -1000;
        }
        
        
        
        if (cliffGameSceneryOrig.data != NULL) {
            //start from cliffGameScenery, not cliffGameSceneryOrig, because we are drawing on cliffGameScenery each time and we want that saved
            cv::resize(cliffGameSceneryOrig, cliffGameScenery, cv::Size(frame_with_mask.cols, frame_with_mask.rows), 0, 0, INTER_LINEAR);
            
            // for text that will be written on top
            if (shouldWriteOnIm) {
                cv::rectangle(cliffGameScenery, textWriteRectangle, textWriteRectangleBackground, -1, 8);
                //cv::line(fieldScenery,cv::Point(0,textWriteRectangle.height),cv::Point(fieldScenery.cols,textWriteRectangle.height),cv::Scalar(0,0,0,255),5);
                cv::line(cliffGameScenery,cv::Point(textWriteRectangle.width,0),cv::Point(textWriteRectangle.width,cliffGameScenery.rows),cv::Scalar(0,0,0,255),5);
            }
            
            
            //Draw cracks
            int numCracks = 10;
            for (; catapultGameCracksDrawn < numCracks*(currentCliffParam-CLIFF_STARTING_PARAM)/(get_max_cliff_param()-CLIFF_STARTING_PARAM); catapultGameCracksDrawn++ ) {
                cout << catapultGameCracksDrawn << endl;
                double diffPercent = 0.1;
                int x1 =   (catapultGameCracksDrawn)*1.0/numCracks*CRACK_AREA_WIDTH *cliff.cols + CRACK_AREA_LEFT_BORDER*cliff.cols;
                int x2 = (1+catapultGameCracksDrawn)*1.0/numCracks*CRACK_AREA_WIDTH *cliff.cols + CRACK_AREA_LEFT_BORDER*cliff.cols;
                int y1, y2;
                if (catapultGameCracksDrawn % 2 == 0) {
                    y1 = (0.5-diffPercent/2) * (CRACK_AREA_HEIGHT*cliff.rows)  + CRACK_AREA_TOP_BORDER*cliff.rows;
                    y2 = (0.5+diffPercent/2) * (CRACK_AREA_HEIGHT*cliff.rows)  + CRACK_AREA_TOP_BORDER*cliff.rows;
                } else {
                    y1 = (0.5+diffPercent/2) * (CRACK_AREA_HEIGHT*cliff.rows)  + CRACK_AREA_TOP_BORDER*cliff.rows;
                    y2 = (0.5-diffPercent/2) * (CRACK_AREA_HEIGHT*cliff.rows)  + CRACK_AREA_TOP_BORDER*cliff.rows;
                }
                y1 -=   (catapultGameCracksDrawn)*0.3/numCracks*CRACK_AREA_HEIGHT *cliff.rows;
                y2 -= (1+catapultGameCracksDrawn)*0.3/numCracks*CRACK_AREA_HEIGHT *cliff.rows;
                
                /*
                //get two random points with in a rectangle defined by the crack area constants
                int x1 =  ( rand() % ((int) (CRACK_AREA_WIDTH *cliff.cols)) ) + CRACK_AREA_LEFT_BORDER*cliff.cols;
                int y1 =  ( rand() % ((int) (CRACK_AREA_HEIGHT*cliff.rows)) ) + CRACK_AREA_TOP_BORDER *cliff.rows;
                int x2 =  ( rand() % ((int) (CRACK_AREA_WIDTH *cliff.cols)) ) + CRACK_AREA_LEFT_BORDER*cliff.cols;
                int y2 =  ( rand() % ((int) (CRACK_AREA_HEIGHT*cliff.rows)) ) + CRACK_AREA_TOP_BORDER *cliff.rows;
                y2 = (y1+y2)/2; //halve the average length of a crack
                x2 = (x1+x2)/2;
                */
                
                //cv::line(cliff, cv::Point(x1,y1), cv::Point(x2,y2), CLR_BLACK_4UC, 2);
                // split channels so we don't draw on the transparent layer
                vector<Mat> channels(4);
                cv::split(cliff, channels); // get the channels (dont forget they follow BGR order in OpenCV)
                cv::line(channels[0], cv::Point(x1,y1), cv::Point(x2,y2), 0, 2); //black line
                cv::line(channels[1], cv::Point(x1,y1), cv::Point(x2,y2), 0, 2); //black line
                cv::line(channels[2], cv::Point(x1,y1), cv::Point(x2,y2), 0, 2); //black line
                //NOTE: Don't draw on transparent layer, channels[3]
                
                cv::merge(channels,cliff);
            }
            int start_x_cliff = shouldWriteOnIm ? textWriteRectangle.width : 0;
            int start_y_cliff = 0;
            if (whichHand == Hand::LEFT) {
                start_x_cliff = cliffGameScenery.cols - cliff.cols-1;
            }
            if (time_diff > gameResultCutoff) { //normal game running
                drawImageOptimized(cliffGameScenery, cliff, start_x_cliff, start_y_cliff);
                
            } else if (currentCliffParam == -1) { //mountain collapsed
                double time_percent = std::min(1.0, time_diff*1.2/gameResultCutoff); //leave 20% of the time for static period after animation (20%->1.2)
                start_y_cliff = time_percent*frame_with_mask.rows;
                drawImageOptimized(cliffGameScenery, cliff, start_x_cliff, start_y_cliff);
                
            } else { //gold nugget animation (actual animation is done on top of hand)
                drawImageOptimized(cliffGameScenery, cliff, start_x_cliff, start_y_cliff);
                
                
            }
            
            
            
            Mat bgmask;
            bitwise_not(fgmask, bgmask);
            cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
            bitwise_and(cliffGameScenery, cliffGameScenery, frame_with_mask, bgmask);
            
            //gold nugget animation (done here so it is on top of hand)
            if (time_diff <= gameResultCutoff && currentCliffParam != -1) {
                if (goldNuggetOrig.data != NULL) {
                    double time_percent = std::min(1.0, time_diff*1.2/gameResultCutoff); //leave 20% of the time for static period after animation (20%->1.2)
                    double max_scale = MAX_NUGGET_SIZE*frame_with_mask.rows*1.0/goldNuggetOrig.rows;
                    double scale = max_scale*time_percent;
                    if (scale > 0) {
                        cv::resize(goldNuggetOrig, goldNugget, cv::Size(), scale, scale);
                        //center the nugget
                        int start_x_nugget = frame_with_mask.cols/2-goldNugget.cols/2;
                        int start_y_nugget = frame_with_mask.rows/2-goldNugget.rows/2;
                        
                        drawImageOptimized(frame_with_mask, goldNugget, start_x_nugget, start_y_nugget);
                    }
                }
            }
            
        }
        
        
        
        
        
        return param;
    }
    
    
#ifdef EXTERN_C
}
#endif
