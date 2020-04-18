//
// Created by benka on 11/25/18.
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
#include <cmath>
#include "fist_analysis.h"
#include "native-lib.h"



using namespace cv;
using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif

// These are declared (with extern modifier) in the fist_analysis.h file, but the values set here.
// Squeeze bottle game
const double HELIUM_TANK_X_LOC = 0.015; //distance of tank edge to wall, percent of entire frame width
const double BALLOON_X_LOC_WITHIN_TANK = 248.0/384;
const double BOTTLE_LID_PERCENTAGE = 39.0/120; //percentage of the width that is covered by the lid/cap
bool updateCup = true;
double percentToFill = 0.0;
cv::Mat bottleGameScenery;
cv::Mat lemonBottle;
cv::Mat cup;
cv::Mat cupResized;

// Pump balloon game
const double BALLOON_GAME_SCENERY_WALL_LEVEL = 600.0/864; // what percent of the height (from the top) does the wall end and the floor begin
const double BALLOON_STARTING_SIZE = 0.045;
double currentBalloonSize = BALLOON_STARTING_SIZE;
cv::Mat balloonGameScenery;
cv::Mat balloon;
cv::Mat heliumTank;
cv::Mat poppedBalloon;
    
#if (MY_OS==MAC_OS || MY_OS==LINUX_OS)
    double lastFistRadius;
    cv::Point lastFistMaxLoc;
#endif


//double min_fist = 2.1, max_fist = 4.0; // TODO: don't normalize with these but find better way

/*
* Takes in a number to be normalized, the minimum that number can be, and its max.
 * Returns it in normalized range (where min = -1, max =1)
*/
/*double normalizeOneAndNegativeOne(double p, double min_p, double max_p) {
    return (2*((p-min_p)/(max_p-min_p))-1);
}*/

#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
extern "C" {

void
Java_com_motracktherapy_motrack_CVActivity_setBottleGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat * imgPtr = (cv::Mat *) addr;
        bottleGameScenery = *imgPtr;
        if (bottleGameScenery.channels() == 4) {
            cv::cvtColor(bottleGameScenery, bottleGameScenery, cv::COLOR_BGRA2RGBA);
        } else if (bottleGameScenery.channels() == 3) {
            cv::cvtColor(bottleGameScenery, bottleGameScenery, cv::COLOR_BGR2RGBA);
        }
        if (whichHand == Hand::LEFT) {
            cv::flip(bottleGameScenery, bottleGameScenery, +1);
        }
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setLemonBottle(JNIEnv *env, jobject, long addrLemonImage, jboolean jincludes_alpha) {
    if (addrLemonImage != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        if (includes_alpha) {
            /* The following is needed for png images with an alpha channel */
            // process input image
            cv::Mat *lemonImage = (cv::Mat *) addrLemonImage;
            // curently image is in bgra. Convert to rgba
            cv::cvtColor(*lemonImage, lemonBottle, cv::COLOR_BGRA2RGBA);
            if (whichHand == Hand::LEFT) {
                cv::flip(lemonBottle, lemonBottle, +1);
            }
            //If you don't want to convert and keep the original colorspace, you can use this
            //lemonBottle = *lemonImage;
        } else {
            /* The following is needed for jpg images or for png images without an alpha channel
             // process input image
             cv::Mat* lemonImage = (cv::Mat*)addrLemonImage;
             // curently image is in bgr, convert to bgra
             Mat lemonBgra(lemonImage->rows, lemonImage->cols, CV_8UC4);
             cv::cvtColor(*lemonImage, lemonBgra, cv::COLOR_BGR2BGRA);
             // rearrange the channels to rgbate
             cv::Mat lemonRgba(lemonBgra.size(), lemonBgra.type());
             int from_to[] = { 0,2, 1,1, 2,0, 3,3 };
             cv::mixChannels(&lemonBgra,1,&lemonRgba,1,from_to,4);
             lemonBottle = setPixelTransparent(lemonRgba,255,255,255);
             */
        }
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setCupImage(JNIEnv *env, jobject, long addrCupImage, jboolean jincludes_alpha) {
    if (addrCupImage != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat *cupImageAddress = (cv::Mat *) addrCupImage;
        cup = *cupImageAddress;
        updateCup = true;
        if (whichHand == Hand::LEFT) {
            cv::flip(cup, cup, +1);
        }
        cupResized = cup.clone();
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setBalloonGameScenery(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat * imgPtr = (cv::Mat *) addr;
        balloonGameScenery = *imgPtr;

        if (balloonGameScenery.channels() == 4) {
            cv::cvtColor(balloonGameScenery, balloonGameScenery, cv::COLOR_BGRA2RGBA);
        } else if (balloonGameScenery.channels() == 3) {
            cv::cvtColor(balloonGameScenery, balloonGameScenery, cv::COLOR_BGR2RGBA);
        }
        
        if (whichHand == Hand::LEFT) {
            cv::flip(balloonGameScenery, balloonGameScenery, +1);
        }
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setBalloon(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        bool includes_alpha = (bool) jincludes_alpha;

        /* The following is needed for png images with an alpha channel */
        // process input image
        cv::Mat *balloonImage = (cv::Mat *) addr;

        int c = balloonImage->channels();
        if (c == 1) { // grayscale
            cv::cvtColor(*balloonImage, balloon,
                         cv::COLOR_GRAY2RGBA);
        } else if (c == 4 || c == 3) {
            cv::cvtColor(*balloonImage, balloon,
                         cv::COLOR_BGRA2RGBA); // curently image is in bgra. Convert to rgba
        }
        
        if (whichHand == Hand::LEFT) {
            cv::flip(balloon, balloon, +1);
        }
    }
}

void
Java_com_motracktherapy_motrack_CVActivity_setHeliumTank(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat *heliumTankImage = (cv::Mat *) addr;
        int c = heliumTankImage->channels();
        if (c == 1) { // grayscale
            cv::cvtColor(*heliumTankImage, heliumTank,
                         cv::COLOR_GRAY2RGBA);
        } else if (c == 4) {
            cv::cvtColor(*heliumTankImage, heliumTank,
                         cv::COLOR_BGRA2RGBA); // curently image is in bgra. Convert to rgba
        }
        
        //*** Don't flip helium tank because it has text

    }
}
void
Java_com_motracktherapy_motrack_CVActivity_setPoppedBalloon(JNIEnv *env, jobject, long addr, jboolean jincludes_alpha) {
    if (addr != -1) {
        bool includes_alpha = (bool) jincludes_alpha;
        cv::Mat *poppedBalloonImage = (cv::Mat *) addr;
        int c = poppedBalloonImage->channels();
        if (c == 1) { // grayscale
            cv::cvtColor(*poppedBalloonImage, poppedBalloon,
                         cv::COLOR_GRAY2RGBA);
        } else if (c == 4) {
            cv::cvtColor(*poppedBalloonImage, poppedBalloon,
                         cv::COLOR_BGRA2RGBA); // curently image is in bgra. Convert to rgba
        }
        if (whichHand == Hand::LEFT) {
            cv::flip(poppedBalloon, poppedBalloon, +1);
        }
    }
}

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */

//Bottle Game
void setBottleGameScenery(cv::Mat picture) {
    if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    bottleGameScenery = picture;
}
void setCupImage(cv::Mat picture) {
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    cup = picture;
    updateCup = true;
    cupResized = cup.clone();
}
void setLemonBottle(cv::Mat picture) {
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    lemonBottle = picture;
}

//Balloon game
void setBalloonGameScenery(cv::Mat picture) {
    if (colorMode == 2) { picture.setTo(CLR_DISABILITY_RED_4UC); }
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    balloonGameScenery = picture;
    
}
void setBalloon(cv::Mat picture) {
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    if (colorMode == 2 && false) {
        cvtColor(picture, picture, COLOR_RGBA2RGB);
        applyColorMap(picture, picture, COLORMAP_WINTER);
        cvtColor(picture, picture, COLOR_RGB2RGBA);
    }
    if (colorMode == 2) {
        /*
        Mat pictureRGB, alphaCh;
        cv::extractChannel(picture, alphaCh, 3);
        cv::cvtColor(picture, pictureRGB, COLOR_RGBA2RGB);
        cv::applyColorMap(pictureRGB, pictureRGB, COLORMAP_WINTER);
        //cv::insertChannel(<#InputArray src#>, <#InputOutputArray dst#>, <#int coi#>)
        Mat tmp[] = {pictureRGB, alphaCh};
        cv::merge(tmp, 2, picture);*/
        
        applyColorMapKeepingAlpha(picture, COLORMAP_WINTER);
    }
    balloon = picture;
}
void setHeliumTank(cv::Mat picture) {
    //don't flip helium tank because helium tank has text on it
    heliumTank = picture;
}
void setPoppedBalloon(cv::Mat picture) {
    if (whichHand == Hand::LEFT) { cv::flip(picture, picture, +1); }
    poppedBalloon = picture;
}

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */

void draw_fist_arrows(cv::Mat &frame_with_mask, int dir, double radius, distInfo d, double ratio) {
    // Will draw the arrows for fist (either pointing outside when dir = 0 or inside when dir =1)
    cv::Scalar color = CLR_MOTRACK_BLUE_4UC; //this is the motrack color, hex #30ABDF
    cv::Scalar outlineClr = Scalar(0,0,0,255);
    double outlineMult = 2;
    double heightMult = 0.015;
    int plusRadX = int((d.maxLoc.x + radius)/ratio);
    int minusRadX = int((d.maxLoc.x - radius)/ratio);
    int plusTwoRadX = int((d.maxLoc.x + 2*radius)/ratio);
    int minusTwoRadX = int((d.maxLoc.x - 2*radius)/ratio);
    int plusRadY = int((d.maxLoc.y + radius)/ratio);
    int minusRadY = int((d.maxLoc.y - radius)/ratio);
    int plusTwoRadY = int((d.maxLoc.y + 2*radius)/ratio);
    int minusTwoRadY = int((d.maxLoc.y - 2*radius)/ratio);

    double tipLength = 0.3;
    if (dir == 0) {// outward arrows
        cv::arrowedLine(frame_with_mask, cv::Point(minusRadX, minusRadY),
                        cv::Point(minusTwoRadX, minusTwoRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult*outlineMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask, cv::Point(plusRadX, minusRadY),
                        cv::Point(plusTwoRadX, minusTwoRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult*outlineMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask, cv::Point(minusRadX, plusRadY),
                        cv::Point(minusTwoRadX, plusTwoRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult*outlineMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask, cv::Point(plusRadX, plusRadY),
                        cv::Point(plusTwoRadX, plusTwoRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult*outlineMult), 8, 0, tipLength);

        cv::arrowedLine(frame_with_mask, cv::Point(minusRadX, minusRadY),
                        cv::Point(minusTwoRadX, minusTwoRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask, cv::Point(plusRadX, minusRadY),
                        cv::Point(plusTwoRadX, minusTwoRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask, cv::Point(minusRadX, plusRadY),
                        cv::Point(minusTwoRadX, plusTwoRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask, cv::Point(plusRadX, plusRadY),
                        cv::Point(plusTwoRadX, plusTwoRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
    } else {// inward arrows
        cv::arrowedLine(frame_with_mask,
                        cv::Point(minusTwoRadX, minusTwoRadY),
                        cv::Point(minusRadX, minusRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult * outlineMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask,
                        cv::Point(plusTwoRadX, minusTwoRadY),
                        cv::Point(plusRadX, minusRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult * outlineMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask,
                        cv::Point(minusTwoRadX, plusTwoRadY),
                        cv::Point(minusRadX, plusRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult * outlineMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask,
                        cv::Point(plusTwoRadX, plusTwoRadY),
                        cv::Point(plusRadX, plusRadY),
                        outlineClr, int(frame_with_mask.rows * heightMult * outlineMult), 8, 0, tipLength);


        cv::arrowedLine(frame_with_mask,
                        cv::Point(minusTwoRadX, minusTwoRadY),
                        cv::Point(minusRadX, minusRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask,
                        cv::Point(plusTwoRadX, minusTwoRadY),
                        cv::Point(plusRadX, minusRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask,
                        cv::Point(minusTwoRadX, plusTwoRadY),
                        cv::Point(minusRadX, plusRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
        cv::arrowedLine(frame_with_mask,
                        cv::Point(plusTwoRadX, plusTwoRadY),
                        cv::Point(plusRadX, plusRadY),
                        color, int(frame_with_mask.rows * heightMult), 8, 0, tipLength);
    }
}


std::tuple<double, cv::Point, double, int> analyze_fist(cv::Mat fgmask, cv::Mat &frame_with_mask, int dir, double ratio, bool include_arrows) {
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
                
                
                if (include_arrows && radius > 0.05*fgmask.cols) {
                    draw_fist_arrows(frame_with_mask, dir,  radius, d, ratio);
                }
                
            }
        }
    }
    
#if (MY_OS==MAC_OS || MY_OS==LINUX_OS)
    lastFistRadius = radius/ratio;
    lastFistMaxLoc = cv::Point(int(d.maxLoc.x/ratio), int(d.maxLoc.y/ratio));
#endif
    
    //return param;
    return std::make_tuple(param, d.maxLoc, radius, highest_y);
}

double do_plain_fist(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
    double param;
    cv::Point dmaxLoc;
    double radius;
    int highest_y;
    std::tie(param, dmaxLoc, radius, highest_y) = analyze_fist(fgmask, frame_with_mask, dir, ratio, true);
    return param;
}
    
    
void drawCup(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
    if (updateCup && cup.data != NULL && cupResized.data != NULL) {
        // Take care of cup
        cv::resize(cup, cupResized, cv::Size(int((frame_with_mask.cols * 0.3)),int((frame_with_mask.rows *0.25))), 0, 0, INTER_LINEAR);
        double cup_fillable_area_top_percent    = 0.0158730159; //=3.0/189 remember that increasing rows goes to the bottom
        double cup_fillable_area_bottom_percent = 0.904761905; //=1 - 18.0/189;
        double cup_filled_up_to_top_percent = cup_fillable_area_bottom_percent - percentToFill*(cup_fillable_area_bottom_percent-cup_fillable_area_top_percent);
        int inner_cup_top = (int) (cup_filled_up_to_top_percent*(cupResized.rows-1));
        int inner_cup_bottom = (int) (cup_fillable_area_bottom_percent* (cupResized.rows-1));

        Mat thresh;
        //in range is inclusive on both sides. 255 alpha means that it is present, not blank at that pixel
        cv::inRange(cupResized, Scalar(230,230,230,255),Scalar(255,255,255,255),thresh);
        // x, y, width, height
        thresh( Rect(0,                 0,  cupResized.cols,    inner_cup_top) ) = 0;
        thresh( Rect(0,  inner_cup_bottom,  cupResized.cols,  cupResized.rows-inner_cup_bottom) ) = 0;
        cupResized.setTo(Scalar(255,255,0,255),thresh); //set to that value only if thresh is equal to 1, not 0
        updateCup = false;
    }
    
    if (cupResized.data != NULL) {
        int width_cup = cupResized.cols;
        int height_cup = cupResized.rows;
        //start x, y happens from top left
        int start_x_cup = (frame_with_mask.cols - width_cup) *0.15;
        if (whichHand == Hand::LEFT) start_x_cup = (frame_with_mask.cols - width_cup) *0.85;
        //int start_x_cup = width_cup*0.5; //make it half a cup width from the left
        int start_y_cup = frame_with_mask.rows -
                          height_cup * 1.25; //make it a quarter a cup height from the bottom
        //cv::resize(cup, cupResized, cv::Size(), 0.75, 0.75);
        //if (width_cup + start_x_cup < frame_with_mask.cols && height_cup + start_y_cup < frame_with_mask.rows && start_x_cup > 0 && start_y_cup > 0) {
        /*copyWithPixelTransparencyUsingAlpha(frame_with_mask, cupResized, start_x_cup,
                                            start_y_cup,
                                            width_cup, height_cup, 250);*/
        drawImageOptimized(frame_with_mask, cupResized, start_x_cup, start_y_cup);
        //}
    }
    
}
    
double do_squeeze_game(cv::Mat& fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
    if (bottleGameScenery.data != NULL) {
        Mat sceneryResized;
        cv::resize(bottleGameScenery, sceneryResized, cv::Size(frame_with_mask.cols, frame_with_mask.rows), 0, 0, INTER_LINEAR);
        Mat bgmask;
        bitwise_not(fgmask, bgmask);
        cv::resize(bgmask, bgmask, frame_with_mask.size(), 0, 0, INTER_NEAREST);

        if (shouldWriteOnIm) {
            // for text that will be written on top
            cv::rectangle(sceneryResized, textWriteRectangle, textWriteRectangleBackground, -1, 8);
            cv::line(sceneryResized,cv::Point(0,textWriteRectangle.height),cv::Point(sceneryResized.cols,textWriteRectangle.height),cv::Scalar(0,0,0,255),5);
        }
        if (colorMode == 2) { frame_with_mask.setTo(CLR_DISABILITY_RED_4UC, bgmask); } //didn't actually test, but I presume this is faster than bitwise_and so let's do this
        else bitwise_and(sceneryResized, sceneryResized, frame_with_mask, bgmask);
    }
    
    double param;
    cv::Point dmaxLoc;
    double radius;
    int highest_y;
    std::tie(param, dmaxLoc, radius, highest_y) = analyze_fist(fgmask, frame_with_mask, dir, ratio, true);
    
    if (!isParamNoExerciseDone(param)) {
        //Draw the bottle
        if (lemonBottle.data != NULL) {
            //Remember: 0 means completely closed and 1 means completely open bottle
            double bottlelevel;
            if (param / 2 < 0.0) {
                bottlelevel = 0.5 - 0.0;
            } else if (param / 2 > 0.5) {
                bottlelevel = 0.5 - 0.5;
            } else {
                bottlelevel = 0.5 - param / 2;
            }
            Mat lemonBottleResized;
            // Set width to the diameter (twice the radius). Then increased it by a bit more to be 3x so it didn't look exactly equal to diameter, which looked too artificial.
            int lemonBottleResizedWidth = (int) ((3 * radius)/ratio);
            int lemonBottleResizedHeight = (int) ((3 * (lemonBottle.rows * radius / lemonBottle.cols))/ratio);
            if (lemonBottleResizedWidth > 0 && lemonBottleResizedHeight > 0) {
                cv::resize(lemonBottle, lemonBottleResized,
                           cv::Size(lemonBottleResizedWidth, lemonBottleResizedHeight),
                           0, 0, INTER_LINEAR);
                int bottle_ht = lemonBottleResized.rows; //66 if unchanged
                int bottle_wd = lemonBottleResized.cols; //120 if unchanged
                double lidHorizOffsetLeft = whichHand == Hand::LEFT ? 0 : bottle_wd*BOTTLE_LID_PERCENTAGE;
                double lidHorizOffsetRight = whichHand == Hand::LEFT ? bottle_wd*BOTTLE_LID_PERCENTAGE : 0;
                for (int col = lidHorizOffsetLeft; col < bottle_wd-lidHorizOffsetRight; col++) {
                    double percent = (col-lidHorizOffsetLeft) / (double) (bottle_wd-lidHorizOffsetLeft-lidHorizOffsetRight);
                    
                    //linear method, the old method before Feb 28th, 2019
                    //int firstToShrink = (int) ((shape0 * percent) * bottlelevel);
                    
                    double mult = 3; //the larger this is, the deeper it penetrates into the bottle. mult of 4 penetrates essentially fully on iPhone XS
                    int firstToShrink = (int) ((bottle_ht * percent*(1-percent)*mult) * bottlelevel);
                    int secondPart = bottle_ht - firstToShrink;
                    
                    for (int row = 0; row < bottle_ht; row++) {
                        if (row < firstToShrink || row > secondPart) {
                            Vec4b &color = lemonBottleResized.at<Vec4b>(row, col); //y,x
                            color[3] = 0; //change the alpha value
                        }
                    }
                    
                    //lemonBottleResized.col(i).setTo(Scalar(255,255,255,127));
                    
                    //lemonBottleResized.col(i).rowRange(0, firstToShrink).setTo(Scalar(255,255,255,127));
                    //lemonBottleResized.col(i).rowRange(second_part, shape1).setTo(Scalar(255,255,255,127));
                    //row = fgmask[0:firstToShrink,i];
                    //cv::Vec4b &pixel = fgmask.at<cv::Vec4b>(y=i, x);
                }
                
                float how_much_left = lidHorizOffsetLeft + (bottle_wd-lidHorizOffsetLeft-lidHorizOffsetRight) * 0.5; //how much of the bottle image should be to the left of d.maxLoc.x (the answer is all of the cap itself, and half the pure bottle, so it looks like the pure bottle is centered)
                int start_x_bottle = dmaxLoc.x/ratio - how_much_left;
                int start_y_bottle = highest_y/ratio - bottle_ht;
                //copyWithPixelTransparencyUsingAlpha(frame_with_mask, lemonBottleResized, start_x_bottle, start_y_bottle, bottle_wd, bottle_ht, 250);
                drawImageOptimized(frame_with_mask, lemonBottleResized, start_x_bottle, start_y_bottle);
            }
            
        }
    }
    

    //Cup image is static (i.e. should be there regardless
    drawCup(fgmask, frame_with_mask, dir, ratio);

    return param;
}


//maximum value before it pops
double get_max_balloon_param() {
    return BALLOON_STARTING_SIZE*10;
}

//minimum value before the balloon can fly away
double get_min_considering_balloon_param() {
    return BALLOON_STARTING_SIZE*3;
}

double calculate_score_increase_for_balloon_game() {
    if (currentBalloonSize != -1) {
        return 10*currentBalloonSize/BALLOON_STARTING_SIZE;
    } else {
        return 0;
    }
}
    
    
//For balloon pump game
double do_balloon_game(cv::Mat fgmask, cv::Mat &frame_with_mask, int dir, double ratio) {
    //std::time_t time_diff = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())-continuousGameResultStartTime;
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration time_span = nowTime - continuousGameResultStartTime;
    double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
    double gameResultCutoff = currentBalloonSize == -1 ? 0.4 : 1.5; //seconds, represents how long the ending game animation goes on

    //std::chrono::steady_clock::duration time_span = nowTime - continuousGameResultStartTime;
    //std::chrono::steady_clock::duration time_span = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - continuousGameResultStartTime);

    //double time_diff = ((double) time_span.count()) * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
    
    
    Mat frame_with_mask_draw = shouldWriteOnIm ? frame_with_mask(Rect(0, textWriteRectangle.height, frame_with_mask.cols, frame_with_mask.rows-textWriteRectangle.height)) : frame_with_mask;



    if (balloonGameScenery.data != NULL) {
        Mat sceneryResized;
        cv::resize(balloonGameScenery, sceneryResized, cv::Size(frame_with_mask.cols, frame_with_mask.rows), 0, 0, INTER_LINEAR);
        Mat bgmask;
        bitwise_not(fgmask, bgmask);
        cv::resize(bgmask, bgmask, frame_with_mask.size(),0,0, INTER_NEAREST );
        // for text that will be written on top
        if (shouldWriteOnIm) {
            cv::rectangle(sceneryResized, textWriteRectangle, textWriteRectangleBackground, -1, 8);
            cv::line(sceneryResized,cv::Point(0,textWriteRectangle.height),cv::Point(sceneryResized.cols,textWriteRectangle.height),cv::Scalar(0,0,0,255),5);
        }
        if (colorMode == 2) { frame_with_mask.setTo(CLR_DISABILITY_RED_4UC, bgmask); } //didn't actually test, but I presume this is faster than bitwise_and so let's do this
        else bitwise_and(sceneryResized, sceneryResized, frame_with_mask, bgmask);
    }

    double param = 0.0;
    continuousGameAnimationState = time_diff > gameResultCutoff ? GAME_STATE_NORMAL_GAMEPLAY : GAME_STATE_CONTINUOUS_GAME_ANIMATION;
    if (continuousGameAnimationState == GAME_STATE_NORMAL_GAMEPLAY) {// if normal gameplay, not ending game animation
        if (time_diff < gameResultCutoff*100) {
            //make continuousGameResultStartTime a really big number so this only happens once (beyond the *100 threshold)
            continuousGameResultStartTime = std::chrono::steady_clock::now() - std::chrono::hours(24);
            currentBalloonSize = BALLOON_STARTING_SIZE;
        }
        /*if (continuousGameResultStartTime != 0) {
            continuousGameResultStartTime = 0;
            currentBalloonSize = BALLOON_STARTING_SIZE;
        }*/
        if (currentBalloonSize == -1) {
            currentBalloonSize = BALLOON_STARTING_SIZE;
        }
        
        cv::Point dmaxLoc;
        double radius;
        int highest_y;
        std::tie(param, dmaxLoc, radius, highest_y) = analyze_fist(fgmask, frame_with_mask, dir, ratio, false);

    } else {
        // -1000 signals to analyzeForFistGame2 that the animation is being done
        // analyzeForFistGame2 converts param to 0 before reporting it higher up though
        param = -1000;
    }

    
    //example crash caused by exception/error
    //cv::copyMakeBorder(sizeRefImgAlpha, sizeRefImgAlpha, 100, 100, -10, 100, BORDER_CONSTANT, Scalar(0,0,0,0));

    
    if (heliumTank.data != NULL) {
        
        int width_tank = heliumTank.cols;
        int height_tank = heliumTank.rows;
        if (width_tank > frame_with_mask_draw.cols*.31 || width_tank < frame_with_mask_draw.cols*.29) {
            cv::resize(heliumTank, heliumTank,
                       cv::Size(frame_with_mask_draw.cols*.3, frame_with_mask_draw.cols*.3*height_tank/width_tank),
                       0, 0, cv::INTER_LINEAR);
            width_tank = heliumTank.cols;
            height_tank = heliumTank.rows;
        }

        //start x, y happens from top left
        int start_x_tank;
        if (whichHand == Hand::LEFT) {
            //Similar to right hand, make the right wall of helium tank HELIUM_TANK_X_LOC percent of the frame away from the right wall of the screen.
            //Then move it even more left to account for the fat that the helium tank nozzle/balloon spawn point isn't in the center of the tank.
            //Otherwise, the spawn point will be too close to the side and thus more of it will be clipped off.
            //The alternative solution would be to just flip the helium tank image. But this doesn't work because this is text on the image.
            start_x_tank = frame_with_mask_draw.cols*(1-HELIUM_TANK_X_LOC)-width_tank-width_tank*BALLOON_X_LOC_WITHIN_TANK;
            
        } else {
            start_x_tank = frame_with_mask_draw.cols*HELIUM_TANK_X_LOC; //left wall of helium tank will be HELIUM_TANK_X_LOC percent of the frame away from the left wall of the screen
        }
        
        //int start_y_tank = frame_with_mask_draw.rows - width_tank * 1.1; //make it 10% of a pic width from the bottom
        //int start_y_tank = frame_with_mask_draw.rows*(1+BALLOON_GAME_SCENERY_WALL_LEVEL)/2 - height_tank; //make the bottom of the tank be halfway between the floor line and the screen bottom
        int start_y_tank = frame_with_mask_draw.rows*BALLOON_GAME_SCENERY_WALL_LEVEL - height_tank/2.0; //make the center of the helium tank the same level as the floor line
        //copyWithPixelTransparencyUsingAlpha(frame_with_mask_draw, heliumTank, start_x_tank, start_y_tank, width_tank, height_tank, 250);
        drawImageOptimized(frame_with_mask_draw, heliumTank, start_x_tank, start_y_tank);

        if (balloon.data != NULL && poppedBalloon.data != NULL) {
            Mat myBalloon;
            if (currentBalloonSize == -1) { //balloon popped
                double popped_balloon_row_ct = frame_with_mask_draw.cols * get_max_balloon_param()*balloon.rows/balloon.cols;
                cv::resize(poppedBalloon, myBalloon,
                           cv::Size(poppedBalloon.cols*1.0/poppedBalloon.rows * popped_balloon_row_ct, popped_balloon_row_ct),
                           0, 0, cv::INTER_NEAREST);
            } else {
                //cv::Size(frame_with_mask_draw.cols * balloonMultiplier, frame_with_mask_draw.cols * balloonMultiplier*balloon.rows/balloon.cols),
                double balloonMultiplier = currentBalloonSize;
                cv::resize(balloon,       myBalloon,
                           cv::Size(frame_with_mask_draw.cols * balloonMultiplier, frame_with_mask_draw.cols * balloonMultiplier*balloon.rows/balloon.cols),
                           0, 0, cv::INTER_NEAREST);
            }
            int width_balloon = myBalloon.cols;
            int height_balloon = myBalloon.rows;
            //start x, y happens from top left
            int start_x_balloon = start_x_tank+width_tank*BALLOON_X_LOC_WITHIN_TANK - 0.5*width_balloon; //center the balloon at 248.0/384th of the way through the tank (248/384 is where the mouth of the tank is located positionwise x)
            int start_y_balloon;
            if (time_diff > gameResultCutoff || currentBalloonSize == -1) { //normal game running or ballon popped
                start_y_balloon = start_y_tank - height_balloon; //make it on top of the tank
                if (currentBalloonSize>=BALLOON_STARTING_SIZE && currentBalloonSize < get_min_considering_balloon_param()) { //draw a line where the balloon would start if it were its max height (but only if it early enough to not make it too easy)
                    double max_balloon_ht = frame_with_mask_draw.cols * get_max_balloon_param()*balloon.rows/balloon.cols;
                    double dash_y = start_y_tank - max_balloon_ht;
                    int thickness = std::max(1.0,frame_with_mask_draw.cols*0.005);
                    double fontScale = getFontScaleFromThickness(thickness);
                    //cv::line(frame_with_mask_draw, Point(start_x_tank,dash_y), Point(start_x_tank+heliumTank.cols,dash_y), CLR_BLACK_4UC, thickness*3);
                    //cv::line(frame_with_mask_draw, Point(start_x_tank,dash_y), Point(start_x_tank+heliumTank.cols,dash_y), CLR_MOTRACK_BLUE_4UC, thickness);
                    Point startDashLine, endDashLine;
                    
                    Point horizSpacing = Point(heliumTank.cols*0.1,0);
                    Point startArrow;
                    
                    string s1 = "Don't go";
                    string s2 = "past here!";
                    Size s1Size = cv::getTextSize(s1, cv::FONT_HERSHEY_DUPLEX, fontScale, fontScale, 0);
                    Size s2Size = cv::getTextSize(s2, cv::FONT_HERSHEY_DUPLEX, fontScale, fontScale, 0);
                    
                    if (whichHand == Hand::LEFT) {
                        startDashLine = cv::Point(start_x_tank+heliumTank.cols*0.5,dash_y);
                        endDashLine   = cv::Point(start_x_tank+heliumTank.cols*1.25,dash_y);
                        drawDashedLine(frame_with_mask_draw, startDashLine, endDashLine, 0.5, 6, CLR_BLACK_4UC, thickness*3);
                        drawDashedLine(frame_with_mask_draw, startDashLine, endDashLine, 0.5, 6, CLR_MOTRACK_BLUE_4UC, thickness);
                        
                        startArrow = Point(start_x_tank-heliumTank.cols*0.0,dash_y);;
                        
                        cv::arrowedLine(frame_with_mask_draw, startArrow, startDashLine-horizSpacing, CLR_BLACK_4UC,      thickness*2*3, LINE_8, 0, 0.3);
                        cv::arrowedLine(frame_with_mask_draw, startArrow, startDashLine-horizSpacing, CLR_MOTRACK_BLUE_4UC, thickness*2, LINE_8, 0, 0.3);
                        
                        cv::putText(frame_with_mask_draw, s1, startArrow-horizSpacing-cv::Point(0,0.1*s1Size.height)-cv::Point(s1Size.width,0), fontType, fontScale, CLR_BLACK_4UC, thickness*3);
                        cv::putText(frame_with_mask_draw, s1, startArrow-horizSpacing-cv::Point(0,0.1*s1Size.height)-cv::Point(s1Size.width,0), fontType, fontScale, CLR_MOTRACK_BLUE_4UC, thickness);
                        cv::putText(frame_with_mask_draw, s2, startArrow-horizSpacing+cv::Point(0,1.1*s2Size.height)-cv::Point(s2Size.width,0), fontType, fontScale, CLR_BLACK_4UC, thickness*3);
                        cv::putText(frame_with_mask_draw, s2, startArrow-horizSpacing+cv::Point(0,1.1*s2Size.height)-cv::Point(s2Size.width,0), fontType, fontScale, CLR_MOTRACK_BLUE_4UC, thickness);
                        
                    } else {
                        startDashLine = cv::Point(start_x_tank+heliumTank.cols*0.0,dash_y);
                        endDashLine   = cv::Point(start_x_tank+heliumTank.cols*1.0, dash_y);
                        drawDashedLine(frame_with_mask_draw, startDashLine, endDashLine, 0.5, 6, CLR_BLACK_4UC, thickness*3);
                        drawDashedLine(frame_with_mask_draw, startDashLine, endDashLine, 0.5, 6, CLR_MOTRACK_BLUE_4UC, thickness);
                        
                        startArrow = Point(start_x_tank+heliumTank.cols*1.4,dash_y);;
                        
                        cv::arrowedLine(frame_with_mask_draw, startArrow, endDashLine+horizSpacing, CLR_BLACK_4UC,      thickness*2*3, LINE_8, 0, 0.3);
                        cv::arrowedLine(frame_with_mask_draw, startArrow, endDashLine+horizSpacing, CLR_MOTRACK_BLUE_4UC, thickness*2, LINE_8, 0, 0.3);
                        
                        cv::putText(frame_with_mask_draw, s1, startArrow+horizSpacing-cv::Point(0,0.2*s1Size.height), fontType, fontScale, CLR_BLACK_4UC, thickness*3);
                        cv::putText(frame_with_mask_draw, s1, startArrow+horizSpacing-cv::Point(0,0.2*s1Size.height), fontType, fontScale, CLR_MOTRACK_BLUE_4UC, thickness);
                        cv::putText(frame_with_mask_draw, s2, startArrow+horizSpacing+cv::Point(0,1.2*s2Size.height), fontType, fontScale, CLR_BLACK_4UC, thickness*3);
                        cv::putText(frame_with_mask_draw, s2, startArrow+horizSpacing+cv::Point(0,1.2*s2Size.height), fontType, fontScale, CLR_MOTRACK_BLUE_4UC, thickness);
                    }

                }
                
            } else { //balloon flying away
                double time_percent = time_diff/gameResultCutoff;
                //make balloon go sinusoidally left to right
                double maxRight = frame_with_mask_draw.cols - width_balloon; //max x value of the left part of the balloon
                double amplitude = (maxRight-start_x_balloon)*time_percent;
                if (amplitude < 0) {
                    amplitude = 0;
                }
                double period = gameResultCutoff*2/3;
                start_x_balloon = start_x_balloon + amplitude*(1-cos(time_diff*2*M_PI/period));
                //make balloon go quadratically up (faster speed as time goes on)
                start_y_balloon = start_y_tank*(1-time_percent*time_percent) - height_balloon;
            }
            //copyWithPixelTransparencyUsingAlpha(frame_with_mask_draw, myBalloon, start_x_balloon, start_y_balloon, width_balloon, height_balloon, 250);
            drawImageOptimized(frame_with_mask_draw, myBalloon, start_x_balloon, start_y_balloon);

        }

    }
    return param;
}

#ifdef EXTERN_C
}
#endif
