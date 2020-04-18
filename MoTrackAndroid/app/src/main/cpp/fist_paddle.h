//
// Created by Rahul Yerrabelli on 02/21/19.
//

#ifndef MOTRACKTHERAPYMOBILE_PADDLE_H
#define MOTRACKTHERAPYMOBILE_PADDLE_H

#include <opencv2/opencv.hpp>
#include "calibration.h"
//#include "all_access_vars.h"


#ifdef EXTERN_C
extern "C" {
#endif
    
    //extern const double CATAPULT_STARTING_PARAM;
    //extern double currentCatapultParam;

#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    /*void
    Java_com_motracktherapy_motrack_CVActivity_setBullseye(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultMount(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setCatapultBeam(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    
    void
    Java_com_motracktherapy_motrack_CVActivity_setFieldScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha);
    */
    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
    
    /*void setFieldScenery(cv::Mat picture);
    void setBullseye(cv::Mat picture);
    void setCatapultMount(cv::Mat picture);
    void setCatapultBeam(cv::Mat picture);
    
    void setCatapultMountBrokenFront(cv::Mat picture);
    void setCatapultMountBrokenBack(cv::Mat picture);
    void setCatapultMountCracked(cv::Mat picture);*/



#endif

std::tuple<double,bool> do_paddle_ball_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio, int);

double calculate_score_increase_for_paddle_game();

class Paddle {

public:

    Paddle();
    Paddle(Point _initPosition, unsigned int _w, unsigned int _h, unsigned int _fw, unsigned int _fh);
    ~Paddle();
    void initializePaddle(Mat& frame_with_mask, int nrtbc_each);
    Point getPosition();
    void setPosition(int _ypos);
    unsigned int getWidth();
    unsigned int getHeight();


private:

    Point position_;
    Point prevPosition_;
    unsigned int width_;
    unsigned int height_;
    unsigned int frameWidth_;
    unsigned int frameHeight_;

};
class Ball{

public:

    Ball();
    Ball(unsigned int _h, unsigned int _w, unsigned int _r, unsigned int _s, Paddle* _lp, Paddle* _rp);
    ~Ball();
    Point getPosition();
    void initializePosition();
    bool updatePosition();
    unsigned int getRadious();
    //int checkWinner();

private:

    Point position_;
    Point prevPosition_;
    unsigned int radius_;
    unsigned int height_;
    unsigned int width_;
    unsigned int speed_;
    Point2f direction_;
    Paddle* lPaddle_;
    Paddle* rPaddle_;
    //int winner_;
};

double analyze_fist(cv::Mat fgmask, cv::Mat &frame_with_mask, int dir, double ratio);


#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_DEVIATION_CATAPULT_H
