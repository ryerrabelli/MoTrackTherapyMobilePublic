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

#include "fist_paddle.h"



using namespace cv;
using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif
    int score_human, score_comp, scoreToPlayTo;
    bool paddleBallInitialized = false;
    cv::Mat emptyMat;
    Paddle humanPaddle;
    Paddle compPaddle;
    Ball ball;
    
    
#if MY_OS==ANDROID_OS
    /*
     * ------------------------------------------------------------------------------------------------
     *                                  JAVA TO CPP INTERFACE FUNCTIONS
     * ------------------------------------------------------------------------------------------------
     */
    extern "C" {

    /*void
    Java_com_motracktherapy_motrack_CVActivity_setPaddleGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat* Address = (cv::Mat*)addr;
        cv::cvtColor(*Address, cliffGameSceneryOrig, cv::COLOR_BGRA2RGBA);
        if (whichHand == Hand::LEFT) { // flip for left
            cv::flip(cliffGameSceneryOrig, cliffGameSceneryOrig, 1);
        }
        cliffGameImagesInitialized = false;
    }*/


    } //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    /*
     * -------------------------------------------------------------------------------------------------
     *                                     iOS INTERFACE FUNCTIONS
     * -------------------------------------------------------------------------------------------------
     */
    
    /*
    void setCliffGameScenery(cv::Mat picture) {
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
    }*/
    
#endif
    /*
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *                                  END OF INTERFACE FUNCTIONS
     * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     */

    void initializePaddle(Mat& frame_with_mask, int nrtbc_each) {
        score_human = 0, score_comp = 0;
        scoreToPlayTo = nrtbc_each;
        humanPaddle = Paddle(Point(20, (int)(frame_with_mask.rows*0.5)), 15, 40, frame_with_mask.cols, frame_with_mask.rows);
        compPaddle = Paddle(Point(frame_with_mask.cols - 20, (int)(frame_with_mask.rows*0.5)), 15, 40, frame_with_mask.cols, frame_with_mask.rows);
        ball = Ball(frame_with_mask.rows, frame_with_mask.cols, 18, 14, &humanPaddle, &compPaddle);
        paddleBallInitialized = true;
        emptyMat = cv::Mat::zeros(frame_with_mask.size(), frame_with_mask.type());
    }

    /*void initializeCliffGameImages(int width, int height) {
        if (cliffGameSceneryOrig.data != NULL) {
            cv::resize(cliffGameSceneryOrig, cliffGameScenery, cv::Size(width, height), 0, 0, INTER_LINEAR);
        }
        if (cliffOrig.data != NULL) {
            cv::resize(cliffOrig, cliff, cv::Size(cliffGameScenery.cols*CLIFF_WIDTH_RATIO, cliffGameScenery.cols));
        }
        
        cliffGameImagesInitialized = true;
    }*/

    double analyze_fist(cv::Mat fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
        distInfo d = get_distance_transform(fgmask);
        double radius = d.dist.at<float>(d.maxLoc.y,d.maxLoc.x);

        //y increases from top to bottom.
        int highest_y = d.maxLoc.y + (int)(radius);

        double param = PARAM_NO_HAND;
        if ( radius > 0.05*std::min(fgmask.cols,fgmask.rows) ) {
            //y increases from top to bottom.
            int lowest_y = -1;
            for (int r = 0; r < fgmask.rows; r++) {
                for (int c = 0; c < fgmask.cols; c++) {
                    if (d.dist.at<float>(r,c) > 0) {
                        lowest_y = r;
                        goto end;
                    }
                }
            }

            end:
            if (lowest_y != -1) {
                param = PARAM_NOT_VALID; //this value is never used (overriden anyway) but kept in case I accidentally introduce a bug later; this way it shows that it is PARAM_NOT_VALID not PARAM_NO_HAND (i.e. there is a hand)

                if (displayExtraInfoAmount > 0) {
                    cv::circle(frame_with_mask, cv::Point(int(d.maxLoc.x/ratio), int(highest_y/ratio)), int(5/ratio), cv::Scalar(255, 0, 255, 255), 1);
                    cv::circle(frame_with_mask, cv::Point(int(d.maxLoc.x/ratio), int( lowest_y/ratio)), int(5/ratio), cv::Scalar(255, 0, 0, 255), 1);
                    //cv::circle(frame_with_mask,d.maxLoc,radius,cv::Scalar(0,0,255, 255));
                    //cv::circle(frame_with_mask,cv::Point(d.maxLoc.x, top_y-radius*3),10,cv::Scalar(0,0,255, 255));
                    cv::line(frame_with_mask, cv::Point(int((d.maxLoc.x-10)/ratio), int((highest_y-radius*3)/ratio)), Point(int((d.maxLoc.x+10)/ratio), int((highest_y-radius*3)/ratio)), Scalar(255,0,255, 255),2);
                }

                // get length/area of wrist
                // old way
                //double param = normalizeOneAndNegativeOne((top_y - bottom_y) / radius, min_fist, max_fist);
                // new way
                int wrist_top_row = highest_y-1, wrist_bottom_row = highest_y+2; // end row is non-inclusive

                if (wrist_top_row < 0 || wrist_bottom_row >= d.dist.rows) {
                    //if fist is too close to bottom and forearm isn't visible
                    //cout << " Fist too low. ";
                    param = PARAM_NOT_VALID;

                } else {
                    int wrist_area = cv::countNonZero( d.dist.rowRange(wrist_top_row, wrist_bottom_row) ); //counts the number of nonzero pixels between start_row (inclusive) and end_row (exclusive)
                    double wrist_len = wrist_area*1.0/(wrist_bottom_row-wrist_top_row);
                    param = (highest_y-lowest_y-radius*3)/wrist_len;
                    char buf[125];
                    snprintf(buf, sizeof(buf), "%.2f", param); //std::to_string(param) is the old way, but can't format it
                    //cv::putText(frame_with_mask, buf , cv::Point(10, 50),cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255, 255), 2);
                    //frame_with_mask = writeScoreFeedback(frame_with_mask, param);




                }
            }
        }

        //return param;
        return param;
    }

    Paddle::Paddle(Point _initPosition, unsigned int _w, unsigned int _h, unsigned int _fw, unsigned int _fh){

        position_ = _initPosition;
        width_ = _w;
        height_ = _h;
        frameWidth_ = _fw;
        frameHeight_ = _fh;

    }

    Paddle::~Paddle(){

    }

    unsigned int Paddle::getWidth(){
        return width_;
    }

    unsigned int Paddle::getHeight(){
        return height_;
    }

    Point Paddle::getPosition(){
        return position_;
    }

    void Paddle::setPosition(int _ypos){

        // The paddle shouldn't go above or below de screen
        if ( ((_ypos + (int)(height_*0.5)) >= frameHeight_) || ((_ypos - (int)(height_*0.5) <= 0)) ){
            return;
        }
        position_.y = _ypos;
    }


Ball::Ball(unsigned int _h, unsigned int _w, unsigned int _r, unsigned int _s, Paddle* _lp, Paddle* _rp){

    height_ = _h;
    width_  = _w;
    radius_ = _r;
    speed_  = _s;
    lPaddle_ = _lp;
    rPaddle_ = _rp;
    initializePosition();
}

Paddle::Paddle(){

}
Ball::Ball(){

}
Ball::~Ball(){

}
Point Ball::getPosition(){
    return position_;
}
    // sets the position of the Ball to the center going in a random direction
    void Ball::initializePosition() {
        //res.winner = 0;// no winner yet
        // The initial position is the center of the screen
        position_ = Point((int)(width_*0.5), (int)(height_*0.5));
        prevPosition_ = Point (0, 0); // this one could be just anything...

        // The direction of the ball is selected randomly
        // But we check that the result is not zero (or close) in any direction
        // because that would give us a ball perpendicular to the walls
        float xdir = (float) (rand()) / ((float)(RAND_MAX/2)) - 1;
        float ydir = (float) (rand()) / ((float)(RAND_MAX/2)) - 1;
        float norm = sqrt(xdir*xdir + ydir*ydir);

        while ( (fabs(xdir/norm) <= 0.1) || (fabs(ydir/norm) <= 0.1)){
            xdir = (float) (rand()) / ((float)(RAND_MAX/2)) - 1;
            ydir = (float) (rand()) / ((float)(RAND_MAX/2)) - 1;
            norm = sqrt(xdir*xdir + ydir*ydir);
        }

        direction_ = Point2f(xdir/norm, ydir/norm);
    }

// returns True if we got a winner, False if not
bool Ball::updatePosition(){

    Point disp((int)(direction_.x * speed_), (int)(direction_.y * speed_));
    Point newPos = position_ + disp;


    // In case it reaches one of the borders
    // If the ball hits either top or bottom borders, it bounces off.

    if ( ((newPos.y + radius_) >= height_) || ((newPos.y -  (int) radius_) <= 0) ){

        direction_.y = -direction_.y;
        disp.x = (int)(direction_.x * speed_);
        disp.y = (int)(direction_.y * speed_);
        newPos = position_ + disp;

    }

    // If the ball hits the paddle
    const int yLmax = lPaddle_->getPosition().y + lPaddle_->getHeight() * 0.5;
    const int yLmin = lPaddle_->getPosition().y - lPaddle_->getHeight() * 0.5;
    const int xLpos = lPaddle_->getPosition().x + lPaddle_->getWidth() * 0.5;

    const int yRmax = rPaddle_->getPosition().y + rPaddle_->getHeight() * 0.5;
    const int yRmin = rPaddle_->getPosition().y - rPaddle_->getHeight() * 0.5;
    const int xRpos = rPaddle_->getPosition().x - rPaddle_->getWidth() * 0.5;

    if ( ((newPos.y - radius_ < yLmax) && ( newPos.y + radius_ > yLmin) && ( (newPos.x - radius_) <= xLpos)) ||
         ((newPos.y - radius_ < yRmax) && ( newPos.y + radius_ > yRmin) && ( (newPos.x + radius_) >= xRpos)) ){

        direction_.x = -direction_.x;
        disp.x = (int)(direction_.x * speed_);
        disp.y = (int)(direction_.y * speed_);
        newPos = position_ + disp;

    }
        // If the ball this one of the lateral walls, the we have a winner
    else if ((newPos.x + radius_) >= width_){
        score_human++;
        initializePosition();
        return true;

    } else if ((newPos.x - (int) radius_) <= 0){
        score_comp++;
        initializePosition();
        return true;
    }
    prevPosition_ = position_;
    position_ = newPos;
    return false;
}

unsigned int Ball::getRadious(){
    return radius_;
}

/*int Ball::checkWinner(){
    return winner_;
}*/
    double minParam = -0.25, maxParam = 0.25;
    std::tuple<double,bool> do_paddle_ball_game(Mat& fgmask, Mat &frame_with_mask, int dir, double ratio, int nrtbc_each) {

        if (!paddleBallInitialized) {
            initializePaddle(frame_with_mask, nrtbc_each);
        }
        Mat bgmask;
        bitwise_not(fgmask, bgmask);
        cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
        bitwise_and(emptyMat, emptyMat, frame_with_mask, bgmask);

        // We draw the ball in the screen
        circle(frame_with_mask, ball.getPosition(), ball.getRadious(), Scalar(0,0,255), -1);
        // get param
        double param = analyze_fist(fgmask,frame_with_mask,dir,ratio);
        // map to position of paddle
        double pos_y = (-param-minParam)*(frame_with_mask.rows/(maxParam-minParam));
        // human paddle is drawn in the screen
        humanPaddle.setPosition(pos_y);
        const Point c1 = humanPaddle.getPosition() - Point(humanPaddle.getWidth()*0.5, humanPaddle.getHeight());
        const Point c2 = humanPaddle.getPosition() + Point(humanPaddle.getWidth()*0.5, humanPaddle.getHeight());
        rectangle(frame_with_mask, c1, c2, Scalar(255,0,0), -1);

        // Computer  paddle is drawn in the screen
        compPaddle.setPosition(ball.getPosition().y);
        const Point c3 = compPaddle.getPosition() - Point(compPaddle.getWidth()*0.5, compPaddle.getHeight());
        const Point c4 = compPaddle.getPosition() + Point(compPaddle.getWidth()*0.5, compPaddle.getHeight());
        rectangle(frame_with_mask, c3, c4, Scalar(255,0,0), -1);

        bool hasWinner = ball.updatePosition();

        /*int winner = ball.checkWinner();
        if (winner == 1){
            score1++;
        } else if (winner == 2){
            score2++;
        }*/

        // write score
        writeText(frame_with_mask, std::to_string(score_human), frame_with_mask.cols*0.25 ,-1 ,-1.0, cv::Scalar(255,255,255));
        writeText(frame_with_mask, std::to_string(score_comp),  frame_with_mask.cols*0.75 ,-1 ,-1.0, cv::Scalar(255,255,255));
        return std::make_tuple(param, hasWinner);
    }

    // the score is the difference between the human and computer score
double calculate_score_increase_for_paddle_game() {
    return score_human-score_comp;
}
#ifdef EXTERN_C
}
#endif
