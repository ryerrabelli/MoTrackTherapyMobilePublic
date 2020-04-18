//
//  simulator.cpp
//  MoTrack Therapy Mac
//
//  Created by Rahul Yerrabelli on 6/21/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include "json.hpp"

#include "simulator.h"

#include "native-lib.h"
#include "calibration.h"
#include "fist_analysis.h"
#include "pronation_analysis.h"
#include "flexion_knock.h"
#include "flexion_cliff.h"
#include "deviation_windshield.h"
#include "deviation_catapult.h"
#include "pronation_dial.h"
#include "pronation_ketchup.h"
#include "audio_player.h"


using namespace std;
using namespace cv;
using namespace nlohmann;


Mat doGrabCut(Mat image) {
    //source: https://stackoverflow.com/questions/17698431/extracting-background-image-using-grabcut
    
    // Open another image
    cv::resize(image, image, cv::Size(), .1, .1);
    
    // define bounding rectangle
    //cv::Rect myRectangle(40,90,image.cols-80,image.rows-170);
    cv::Rect myRectangle(image.cols*0.12,image.rows*.43,image.cols*0.65,image.rows);
    cout << "ht: " << image.rows << " wt: " <<image.cols << endl;
    
    cv::Mat result; // segmentation result (4 possible values)
    cv::Mat bgModel,fgModel; // the models (internally used)
    
    // GrabCut segmentation
    cv::grabCut(image,    // input image
                result,   // segmentation result
                myRectangle,// rectangle containing foreground
                bgModel,fgModel, // models
                1,        // number of iterations
                cv::GC_INIT_WITH_RECT); // use rectangle
    
    // Get the pixels marked as likely foreground
    cv::compare(result,cv::GC_PR_FGD,result,cv::CMP_EQ);
    // Generate output image
    cv::Mat foreground(image.size(),CV_8UC3,cv::Scalar(255,255,255));
    //cv::Mat background(image.size(),CV_8UC3,cv::Scalar(255,255,255));
    image.copyTo(foreground,result); // bg pixels not copied
    
    // draw rectangle on original image
    cv::rectangle(image, myRectangle, cv::Scalar(255,255,255),1);
    
    //cv::resize(image, image, cv::Size(), 10, 10);
    //cv::resize(foreground, foreground, cv::Size(), 10, 10);
    cv::imshow("input", image);
    cv::imshow("fg", foreground);
    return foreground;
}

void testCircularCharacteristics() {
    Mat mymat = Mat::eye( 10, 10, CV_8UC1 )*10+175;
    cv::subtract(mymat, 180, mymat, (mymat>180));
    
    double avg, stdev;
    std::tie(avg, stdev) = circularMeanStdDev(mymat,0,180);
    cout << "avg: " << avg << ", stdev: " << stdev << ", channels: " << mymat.channels() <<endl;
    
    Scalar mean_arith, stdev_arith;
    cv::meanStdDev(mymat, mean_arith, stdev_arith);
    cout << "avg: " << mean_arith << ", stdev: " << stdev_arith << ", channels: " << mymat.channels() << endl;
    
}


//This runs the code as if you were playing it on the phone (or can run a video as well)
int playGame() {
    if (false) {
        Mat image= cv::imread("ExampleHandImage.png");
        Mat output = doGrabCut(image);
        cv::waitKey(0);
    }
    
    //std::string videoName = "IMG_5145 trimmed.MOV"; //alg 1600 & 1610 work on this, Apr 13, 2019 (except for a split second), but need to line redbox appropriately, try a waitkey of 65msec
    //std::string videoName = ""; //empty string indicates to use the camera
    //std::string videoName = "videos/2019-03-08 11.47.44.mp4";
    std::string videoName = "videos/2019-06-11 01.07.mp4"; //this is a video where normal algorithm 4/0 is much smaller than what it should be
    
    //std::string videoName = "videos/2019-06-11 01.07.mp4";
    
    int nrtbc_each = 4;
    int hand = 0;
    int exerciseGame = FIST_SQUEEZE_BOTTLE_GAME;
    int start_time_msec = 0000; //2500;
    
    
    
    //SET UP ORIENTATION
    int orientationGame = getOrientationForExerciseGame( exerciseGame );
    setExerciseGameAndOrientation(exerciseGame, orientationGame);
    
    
    
    //INITIALIZE C++ WITH THE NUMBER OF REPS AND THE HAND
    //initialize((int) nrtbc_each, (int) hand);
    initialize((int) nrtbc_each, (int) hand, "{ \"segmentationAlg\": [3000], \"colorMode\":0, , \"segmentationFrameDelayAmt\": 1 }");
    
    
    //SET UP IMAGES (ON ANDROID, WOULD SET UP AUDIO, BUT NO NEED TO SET UP AUDIO ON iOS)
    std::string pathToImages = "../../MoTrackAndroid/app/src/main/res/drawable/";
    setupImages(exercise_game, pathToImages);
    
    
    
    //SET UP DEVICE-SPECIFIC USER INTERFACE
    VideoCapture cap;
    if (videoName == "") cap = VideoCapture(0);
    else cap = VideoCapture(videoName);
    if(!cap.isOpened()) { // check if we succeeded
        cout << "Camera couldn't be opened." << endl;
        return -1;
    }
    
    bool success = cap.set(CAP_PROP_POS_MSEC, start_time_msec);
    cout << "successfully set start time: " << success << endl;
    
    namedWindow("input", WINDOW_NORMAL);
    namedWindow("relevant input", WINDOW_NORMAL);
    namedWindow("output", WINDOW_NORMAL);
    
    /*
    while(true) {
        Mat frame;
        cap >> frame; // get a new frame from camera, is 3 channels
        double initScale = 300.0/frame.cols;
        cv::resize(frame, frame, cv::Size(), initScale, initScale);
        cv::imshow("output", frame);
        
        char key = (char) cv::waitKey(1); //cast can be done implicitly, but showed for clarification
        if (key == 27 || key == 'q' || key == 'b') break;
        else if (key == 'r') resetCalibrationBackground();
        else if (key == 's') {
            doSkip();
            cout << "Skip" << endl;
        }
        else if (key == ' ') cv::waitKey(0); //pause
    }//*/
    
    
    Mat frame;
    while(true) {
        cap >> frame; // get a new frame from camera, is 3 channels
        if (frame.data == NULL) {
            cv::waitKey(0); //quit when user presses a button
            break;
        }
        double initScale = 300.0/frame.cols;
        cv::resize(frame, frame, cv::Size(), initScale, initScale);

        cv::flip(frame, frame, +1); //flip left/right because front camera is given as a mirror image
        //frame = frame(Rect(frame.cols*0.1, frame.rows*0.1, frame.cols*0.9, frame.rows*0.9));
        cv::Mat frameRelevant;
        cv::cvtColor(frame, frameRelevant, COLOR_BGR2RGBA); //make it four channels

        int doneState = getDoneState();
        if (doneState > 0) {
            break; //end program, finished successfully

        } else if (doneState < 0) {
            std::string errorDescript      = getCrashErrorDescription();
            std::string errorDescriptCoded = getCrashErrorDescriptionCoded();
            cout << "ERROR: " << endl;
            cout << errorDescript << endl;
            break;

        } else {

        int height;
        int width;
        double aspect_ratio = 4.0/3.0;

        if (orientationGame == 0) {//portrait
            if (videoName != "" && frameRelevant.rows < frameRelevant.cols) {
                cv::rotate(frameRelevant, frameRelevant, cv::ROTATE_90_CLOCKWISE);
                cv::flip(frameRelevant, frameRelevant, -1);
                }
                height = frameRelevant.rows;
                width = height/aspect_ratio;
                if (width > frameRelevant.cols) {
                width = frameRelevant.cols;
                height = width*aspect_ratio;
            }

        } else { //landscape
            if (videoName != "" && frameRelevant.rows > frameRelevant.cols) {
                cv::rotate(frameRelevant, frameRelevant, cv::ROTATE_90_CLOCKWISE);
                cv::flip(frameRelevant, frameRelevant, -1);
            }
            height = frameRelevant.rows;
            width = height*aspect_ratio;
            if (width > frameRelevant.cols) {
            width = frameRelevant.cols;
            height = width/aspect_ratio;
            }
        }

        Rect myRect = Rect(frameRelevant.cols/2-width/2, frameRelevant.rows-height, width, height);
        frameRelevant = frameRelevant(myRect);

        Mat output, outputMask;
        std::tie(output, outputMask) = inputImage(frameRelevant.clone(), -1, -1,  false);

            cv::cvtColor(frameRelevant, frameRelevant, COLOR_BGR2RGB);
            cv::cvtColor(output, output, COLOR_BGRA2RGBA);
        double scale = sqrt(500*500*1.0/(output.cols*output.rows));
        cv::resize(output, output, cv::Size(),scale,scale);
        cv::resize(frame, frame, cv::Size(),scale,scale);
        cv::resize(frameRelevant, frameRelevant, cv::Size(),scale,scale);
        cv::imshow("input", frame);
        cv::imshow("relevant input", frameRelevant);
        cv::imshow("output", output);

        char key = (char) cv::waitKey(1); //cast can be done implicitly, but showed for clarification
        if (key == 27 || key == 'q' || key == 'b') break;
        else if (key == 'r') resetCalibrationBackground();
        else if (key == 's') {
            doSkip();
            cout << "Skip" << endl;
        } else if (key == ' ') cv::waitKey(0); //pause
            std::string soundName = getSoundToPlayIfAny();
            playSoundIfAny(soundName);
        }

    }//*/
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
    
}


cv::Mat getImage(std::string path) {
    cv::Mat image = cv::imread( path, IMREAD_COLOR ); // Read the file
    if (image.data == NULL) cout << "image at " + path + " is null" << endl;
    //cout << "image at " + path + " has " << image.channels() << " channels" << endl;
    
    //convert from 3 channel to 4 channel way (very inefficient, but works for now since it is only done once total per image, not every frame)
    if (image.rows >0 && image.cols > 0 && image.channels() == 3) {
        cv::Mat imgWithAlpha(image.rows, image.cols, CV_8UC4);
        cv::cvtColor(image, imgWithAlpha, COLOR_BGR2RGBA);
        
        // find all black pixel and set alpha value to zero:
        for (int y = 0; y < imgWithAlpha.rows; ++y) {
            for (int x = 0; x < imgWithAlpha.cols; ++x) {
                cv::Vec4b & pixel = imgWithAlpha.at<cv::Vec4b>(y, x);
                // if pixel is black
                if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
                    // set alpha to zero:
                    pixel[3] = 0;
                }
            }
        }
        
        //cv::imshow("original", image);
        //cv::imshow("new", imgWithAlpha);
        return imgWithAlpha;
    } else return image;
}

void setupImages(int exercise_game, std::string pathToImages) {
    if (exercise_game == FIST_SQUEEZE_BOTTLE_GAME) { //squeeze bottle game
        std::string str0 = pathToImages+"squeezegame_scenery.png";
        setBottleGameScenery( getImage(str0) );
        
        std::string str1 = pathToImages+"squeezegame_cup.png";
        setCupImage( getImage(str1) );
        
        std::string str2 = pathToImages+"squeezegame_lemonbottle.png";
        setLemonBottle( getImage(str2) );
        
        
    } else if (exercise_game == FIST_PUMP_BALLOON_GAME) { //balloon game
        std::string str;
        str = pathToImages+"balloongame_scenery.png";
        setBalloonGameScenery( getImage(str) );
        
        str = pathToImages+"balloongame_balloon.png";
        setBalloon( getImage(str) );
        
        str = pathToImages+"balloongame_heliumtank.png";
        setHeliumTank( getImage(str) );
        
        str = pathToImages+"balloongame_poppedballoon.png";
        setPoppedBalloon( getImage(str) );
        
    } else if (exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME) { //windshield wipe
        std::string str;
        str = pathToImages+"windshieldgame_carframe.png";
        Mat img = getImage(str);
        setCarFrame( getImage(str) );
        
        str = pathToImages+"windshieldgame_scenery.png";
        setWindshieldGameScenery( getImage(str) );
        
        str = pathToImages+"windshieldgame_windshieldbottom.png";
        setWindshieldWiperBottom( getImage(str) );
        
        str = pathToImages+"windshieldgame_windshieldtop.png";
        setWindshieldWiperTop( getImage(str) );
        
        
    } else if (exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME) { //catapult launch
        std::string str;
        str = pathToImages+"catapultgame_scenery.png";
        setFieldScenery( getImage(str) );
        
        str = pathToImages+"catapultgame_bullseye.png";
        setBullseye( getImage(str) );
        
        str = pathToImages+"catapultgame_catapultbeam.png";
        setCatapultBeam( getImage(str) );
        
        str = pathToImages+"catapultgame_catapultmount.png";
        setCatapultMount( getImage(str) );
        
        str = pathToImages+"catapultgame_catapultmountbrokenfront.png";
        setCatapultMountBrokenFront( getImage(str) );
        
        str = pathToImages+"catapultgame_catapultmountbrokenback.png";
        setCatapultMountBrokenBack( getImage(str) );
        
        str = pathToImages+"catapultgame_catapultmountcracked.png";
        setCatapultMountCracked( getImage(str) );
        
        
    } else if (exercise_game == FLEXION_DOOR_KNOCK_GAME) { //door knock game
        std::string str = pathToImages+"knockgame_scenery.png";
        setKnockGameScenery( getImage(str) );
        
        
    } else if (exercise_game == FLEXION_CLIFF_MINE_GAME) { //cliff game
        std::string str;
        str = pathToImages+"cliffgame_scenery.png";
        setCliffGameScenery( getImage(str) );
        
        str = pathToImages+"cliffgame_cliff.png";
        setCliffGameCliff( getImage(str) );
        
        str = pathToImages+"cliffgame_goldnugget.png";
        setCliffGameGoldNugget( getImage(str) );
        
        
    } else if (exercise_game == PRONATION_TURN_DIAL_GAME) { //dial turn
        std::string str;
        str = pathToImages+"dialgame_dialframe.png";
        setDialFrame( getImage(str) );
        
        str = pathToImages+"dialgame_dialpointer.png";
        setDialPointer( getImage(str) );
        
        str = pathToImages+"dialgame_ovenbackground";
        setOvenBackground( getImage(str) );
        
    }
    
    
    if (exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME || exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME || exercise_game == PRONATION_TURN_DIAL_GAME) { //windshield, catapult, door knock, and dial game
        std::string str;
        str = pathToImages+"sizeref_openpalm.png";
        setSizeRefImg( getImage(str) );
        
    } else if (exercise_game == FIST_SQUEEZE_BOTTLE_GAME || exercise_game == FIST_PUMP_BALLOON_GAME) { //squeeze bottle and pump balloon
        std::string str;
        str = pathToImages+"sizeref_closedfist.png";
        setSizeRefImg( getImage(str) );
        
    } else if (exercise_game == FLEXION_DOOR_KNOCK_GAME) {
        std::string str;
        str = pathToImages+"sizeref_flexionextension.png";
        setSizeRefImg( getImage(str) );
    }
    
    
    std::string str_left  = pathToImages+"left_curve_arrow.png";
    std::string str_right = pathToImages+"right_curve_arrow.png";
    setCurveArrows( getImage(str_left) , getImage(str_right));
    
}


void playSoundIfAny(std::string soundName) {
    if (soundName != "" && soundName != "_") {
        //haven't made a way to play sound yet.
    }
}
