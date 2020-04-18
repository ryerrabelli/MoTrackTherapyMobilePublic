//
//  OpenCVWrapper.mm
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 12/25/18.
//  Recreated by Rahul Yerrabelli on 2/13/19.
//  Copyright © 2018/2019 MoTrack Therapy. All rights reserved.
//

//Figured out how to do this with: https://medium.com/@borisohayon/ios-opencv-and-swift-1ee3e3a5735b
/*
 "In order to use the C++ framework, we have to change to Objective-C++. This is done by simply changing the extension of OpenCVWrapper.m to OpenCVWrapper.mm. We can then import the library in our .mm file"
 */

#import <opencv2/opencv.hpp>
//below import is needed for UIImageToMat and MatToUIImage functions
#import <opencv2/imgcodecs/ios.h>


#import "OpenCVWrapper.h"
#include <iostream>
#import <AVFoundation/AVFoundation.h>


#import "native-lib.h"
#import "fist_analysis.h"
#import "hook_fist_analysis.h"
#import "pronation_analysis.h"
#import "flexion_knock.h"
#import "flexion_cliff.h"
#import "deviation_windshield.h"
#import "deviation_catapult.h"
#import "pronation_dial.h"
#import "pronation_ketchup.h"
#import "fabduction_peacock.h"
#import "topposition_alligator.h"
#import "hook_fist_caterpillar.h"
#import "audio_player.h"


using namespace std;
using namespace cv;


@interface OpenCVWrapper () <AVAudioPlayerDelegate>

@end



@implementation OpenCVWrapper

- (void) isThisWorking {
    cout << "Hey" << endl;
}

- (void) isThisWorkingNum:(int)mynum; {
    cout << "A number" << endl;
    if (mynum > 5) {
        cout << "Greater than 5" << endl;
    }
}

+ (cv::Mat) testOpenCV:(cv::Mat)img; {
    cv::putText(img,"Hello from OpenCV", cv::Point(50,150), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,0,0), 2);
    return img;
}


+ (cv::Mat) getMatFrom:(NSString*) name {
    return [self getMatFrom:name andAlphaExist:true];
}

+ (cv::Mat) getMatFrom:(NSString*) name andAlphaExist:(bool) alphaExist {
    UIImage *knockImage =  [UIImage imageNamed:name];
    cv::Mat mymat;
    UIImageToMat(knockImage, mymat, alphaExist); //andAlphaExist is optional parameter, with default false. If false, still returns 4 channels but fourth channel is all 255
    return mymat;
}

+ (int) getDoneStateObjC {
    return getDoneState();
}

+ (NSString*) getCrashErrorDescriptionObjC {
    std::string cppOutput = getCrashErrorDescription();
    NSString* str = [NSString stringWithUTF8String:cppOutput.c_str()];
    return str;
    //alternatively, can make NSString directly here using literals like: return @"windshield_wiper"
}
+ (NSString*) getCrashErrorDescriptionCodedObjC {
    std::string cppOutput = getCrashErrorDescriptionCoded();
    NSString* str = [NSString stringWithUTF8String:cppOutput.c_str()];
    return str;
    //alternatively, can make NSString directly here using literals like: return @"windshield_wiper"
}

+ (void) playAudioFileClickObjC {
    playAudioFileClick();
}
+ (void) playAudioFileCorrectObjC {
    playAudioFileCorrect();
}
+ (void) playAudioFileDoorKnockbjC {
    playAudioFileDoorknock();
}
+ (void) playAudioFileEatingObjC {
    playAudioFileEating();
}
+ (void) playAudioFileCameraObjC {
    playAudioFileCamera();
}


+ (void) setupImagesFor:(int)exercise_game {
    if (exercise_game == FIST_SQUEEZE_BOTTLE_GAME) { //squeeze bottle game
        NSString * str0 = @"squeezegame_scenery.png";
        Mat img0 = [self  getMatFrom:str0];
        setBottleGameScenery(img0);

        NSString * str1 = @"squeezegame_cup.png";
        setCupImage([self  getMatFrom:str1]);

        NSString * str2 = @"squeezegame_lemonbottle.png";
        setLemonBottle([self  getMatFrom:str2]);


    } else if (exercise_game == FIST_PUMP_BALLOON_GAME) { //balloon game
        NSString * str0 = @"balloongame_scenery.png";
        Mat img0 = [self  getMatFrom:str0];
        setBalloonGameScenery(img0);

        NSString * str1 = @"balloongame_balloon.png";
        Mat img1 = [self  getMatFrom:str1];
        setBalloon(img1);

        NSString * str2 = @"balloongame_heliumtank.png";
        Mat img2 = [self  getMatFrom:str2];
        setHeliumTank(img2);

        NSString * str3 = @"balloongame_poppedballoon.png";
        Mat img3 = [self  getMatFrom:str3];
        setPoppedBalloon(img3);


    } else if (exercise_game == FIST_PADDLE_BALL_GAME) { //paddle ball game
        NSString * str0 = @"balloongame_scenery.png";
        Mat img0 = [self  getMatFrom:str0];
        setBalloonGameScenery(img0);
        
        NSString * str1 = @"balloongame_balloon.png";
        Mat img1 = [self  getMatFrom:str1];
        setBalloon(img1);
        
        NSString * str2 = @"balloongame_heliumtank.png";
        Mat img2 = [self  getMatFrom:str2];
        setHeliumTank(img2);
        
        NSString * str3 = @"balloongame_poppedballoon.png";
        Mat img3 = [self  getMatFrom:str3];
        setPoppedBalloon(img3);
        
        
    } else if (exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME) { //windshield wipe
        NSString * str1 = @"windshieldgame_carframeandwheel.png";
        setCarFrame([self  getMatFrom:str1]);

        NSString * str2 = @"windshieldgame_scenery.png";
        setWindshieldGameScenery([self  getMatFrom:str2 andAlphaExist: false]);

        NSString * str3 = @"windshieldgame_windshieldbottom.png";
        setWindshieldWiperBottom([self  getMatFrom:str3]);

        NSString * str4 = @"windshieldgame_windshieldtop.png";
        setWindshieldWiperTop([self  getMatFrom:str4]);


    } else if (exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME) { //catapult launch
        cv::Mat mat1 = [self  getMatFrom:@"catapultgame_scenery.png"] ;
        setFieldScenery(mat1);

        cv::Mat mat2 = [self  getMatFrom:@"catapultgame_bullseye.png"] ;
        setBullseye(mat2);

        cv::Mat mat3 = [self  getMatFrom:@"catapultgame_catapultbeam.png"] ;
        setCatapultBeam(mat3);

        cv::Mat mat4 = [self  getMatFrom:@"catapultgame_catapultmount.png"] ;
        setCatapultMount(mat4);

        cv::Mat mat5 = [self  getMatFrom:@"catapultgame_catapultmountbrokenfront.png"] ;
        setCatapultMountBrokenFront(mat5);

        cv::Mat mat6 = [self  getMatFrom:@"catapultgame_catapultmountbrokenback.png"] ;
        setCatapultMountBrokenBack(mat6);

        cv::Mat mat7 = [self  getMatFrom:@"catapultgame_catapultmountcracked.png"] ;
        setCatapultMountCracked(mat7);

        
    } else if (exercise_game == FLEXION_DOOR_KNOCK_GAME) { //door knock game
        NSString * str = @"knockgame_scenery.png";
        setKnockGameScenery([self  getMatFrom:str andAlphaExist: false]);
        
        
    } else if (exercise_game == FLEXION_CLIFF_MINE_GAME) { //cliff mine game
        NSString * str1 = @"cliffgame_scenery.png";
        setCliffGameScenery([self  getMatFrom:str1 andAlphaExist: false]);
        
        NSString * str2 = @"cliffgame_cliff.png";
        setCliffGameCliff([self  getMatFrom:str2 andAlphaExist: true]);
        
        NSString * str3 = @"cliffgame_goldnugget.png";
        setCliffGameGoldNugget([self  getMatFrom:str3 andAlphaExist: true]);
        
        
    } else if (exercise_game == PRONATION_TURN_DIAL_GAME) { //dial turn
        NSString * str1 = @"dialgame_dialframe.png";
        //NSString * str1 = @"dialgame_exampledial.png";
        setDialFrame([self  getMatFrom:str1]);

        NSString * str2 = @"dialgame_dialpointer.png";
        setDialPointer([self  getMatFrom:str2]);

        NSString * str3 = @"dialgame_ovenbackground";
        setOvenBackground([self  getMatFrom:str3 andAlphaExist: false]);

    } else if (exercise_game == PRONATION_KETCHUP_SHAKE_GAME) { //ketchup shake game
        Mat mat1 = [self  getMatFrom:@"ketchupgame_scenery.png"];
        setKetchupGameScenery(mat1);
        
        Mat mat2 = [self  getMatFrom:@"ketchupgame_platewithfries1.png"];
        setKetchupGamePlateWithFries1(mat2);
        
        Mat mat3 = [self  getMatFrom:@"ketchupgame_platewithfries2.png"];
        setKetchupGamePlateWithFries2(mat3);
        
        Mat mat4 = [self  getMatFrom:@"ketchupgame_bottle.png"];
        setKetchupGameKetchupBottle(mat4);
        
        Mat mat5 = [self  getMatFrom:@"ketchupgame_bottlecap.png"];
        setKetchupGameKetchupBottleCap(mat5);
        
        Mat mat6 = [self  getMatFrom:@"ketchupgame_singlefrywithoutketchup.png"];
        setKetchupGameSingleFryWithoutKetchup(mat6);
        
        Mat mat7 = [self  getMatFrom:@"ketchupgame_singlefrywithketchup.png"];
        setKetchupGameSingleFryWithKetchup(mat7);
        
    } else if (exercise_game == FABDUCTION_PEACOCK_GAME) {
        Mat mat1 = [self  getMatFrom:@"peacockgame_scenery"];
        setPeacockGameScenery(mat1);
        
        Mat mat2 = [self  getMatFrom:@"peacockgame_peacockbody.png"];
        setPeacockGamePeacockBody(mat2);
        
        Mat mat3 = [self  getMatFrom:@"peacockgame_feather.png"];
        setPeacockGameFeather(mat3);
        
    } else if (exercise_game == TOPPOSITION_ALLIGATOR_GAME) {
        Mat mat1 = [self  getMatFrom:@"alligatorgame_alligatorbody.png"];
        setAlligatorGameAlligatorBody(mat1);
        
        Mat mat2 = [self  getMatFrom:@"alligatorgame_alligatormouthupper.png"];
        setAlligatorUpperMouth(mat2);
        
        Mat mat3 = [self  getMatFrom:@"alligatorgame_alligatormouthlower.png"];
        setAlligatorLowerMouth(mat3);
        
    } else if (exercise_game == HOOK_FIST_CATERPILLAR_GAME) {
        Mat mat1 = [self  getMatFrom:@"caterpillargame_scenery.png"];
        setCaterpillarGameScenery(mat1);
        
    }

    
    NSString * strSizeRef;
    if (exercise_game == DEVIATION_PLAIN_GAME || exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME || exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME || exercise_game == PRONATION_PLAIN_GAME || exercise_game == PRONATION_TURN_DIAL_GAME || exercise_game == PRONATION_KETCHUP_SHAKE_GAME) {
        strSizeRef = @"sizeref_openpalm.png";
        setSizeRefImg([self  getMatFrom:strSizeRef]);

    } else if (exercise_game == FIST_PLAIN_GAME || exercise_game == FIST_SQUEEZE_BOTTLE_GAME || exercise_game == FIST_PUMP_BALLOON_GAME || exercise_game == FIST_PADDLE_BALL_GAME) { //squeeze bottle and pump balloon (and eventually FIST_PADDLE_BALL_GAME)
        strSizeRef = @"sizeref_closedfist.png";
        cv::Mat m  = [self  getMatFrom:strSizeRef];
        setSizeRefImg(m);

    } else if (exercise_game == HOOK_FIST_PLAIN_GAME || exercise_game == HOOK_FIST_CATERPILLAR_GAME) {
        strSizeRef = @"sizeref_hookfist";
        cv::Mat m  = [self  getMatFrom:strSizeRef];
        setSizeRefImg(m);
        
    } else if (exercise_game == FLEXION_PLAIN_GAME || exercise_game == FLEXION_DOOR_KNOCK_GAME || exercise_game == FLEXION_CLIFF_MINE_GAME) { //door knock and cliff mine
        strSizeRef = @"sizeref_flexionextension.png";
        setSizeRefImg([self  getMatFrom:strSizeRef]);
        
    } else if (exercise_game == EXERCISE_GAME_ABDUCTION_PLAIN || exercise_game == FABDUCTION_PEACOCK_GAME) {
        strSizeRef = @"sizeref_openpalm.png";
        setSizeRefImg([self  getMatFrom:strSizeRef]);
        
    } else if (exercise_game == EXERCISE_GAME_OPPOSITION_PLAIN || exercise_game == TOPPOSITION_ALLIGATOR_GAME) {
        strSizeRef = @"sizeref_thumb.png";
        setSizeRefImg([self  getMatFrom:strSizeRef]);
        
    }

    if (exercise_game == DEVIATION_PLAIN_GAME || exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME || exercise_game == FLEXION_PLAIN_GAME || exercise_game == FLEXION_DOOR_KNOCK_GAME) {
        // get feedback arrows for radial and ulnar deviation
        NSString * str_left  = @"left_curve_arrow.png";
        NSString * str_right = @"right_curve_arrow.png";
        setCurveArrows([self  getMatFrom:str_left], [self  getMatFrom:str_right]);
    }
}

//ObjC's NSInteger gets wrapped to swift's int. ObjC's int gets wrapped to swift's int32. See https://stackoverflow.com/questions/28102927/cannot-call-an-objc-method-taking-int-in-swift-because-int32-cannot-be-converted
+ (void) initializeObjcWithReps:(NSInteger)nrtbc_each andHand:(NSInteger)hand andJSONDataStr:(NSString*) json_data_str {
    cout << "Hey, initializeObjc" << endl;
    
    //For ML algorithms, get the necessary files
    //Source: https://stackoverflow.com/questions/43504172/opencv-read-files-in-swift-ios-project
    NSBundle* mainBundle;
    NSString *pathPb = [[NSBundle mainBundle] pathForResource:@"tensorflowModel" ofType:@"pb"];
    NSString *pathPbTxt = [[NSBundle mainBundle] pathForResource:@"tensorflowModel.pbtxt" ofType:@"txt"];
    std::string pathPbStr    = std::string([pathPb    cStringUsingEncoding:NSUTF8StringEncoding]);
    std::string pathPbTxtStr = std::string([pathPbTxt cStringUsingEncoding:NSUTF8StringEncoding]);
    cout << pathPbStr << "; " << pathPbTxtStr << endl;
    //char *pathCString = [path cStringUsingEncoding:NSUTF8StringEncoding];
    
    //NOTE: Don't know if I should convert json_data_str using NSUTF8StringEncoding instead of UTF8String. It was working fine as UTF8String so I don't want to change since it isn't broken, but in the comments of this, it recommends you do https://stackoverflow.com/a/8001703
    initialize((int) nrtbc_each, (int) hand, [json_data_str UTF8String], pathPbStr, pathPbTxtStr);
}

+ (int) getOrientationObjC:(int)exercise_game {
    return getOrientationForExerciseGame(exercise_game);
}

+ (bool) shouldCameraBeInitializedAsContinuousObjC {
    return shouldCameraBeInitializedAsContinuous(exercise_game);
}

+ (void) setExerciseGame:(int)exercise_game andOrientationGame:(int) orientation_game {
    setExerciseGameAndOrientation(exercise_game,orientation_game);

    //[self setupImagesFor:exercise_game]; //call the setupImagesFor method
}


+ (void) setShowBackground:(int)n_showbackground {
    showbackground = n_showbackground;
}

+ (int) incrementShowBackground {
    if (showbackground < 2) {
        showbackground = showbackground + 1;
    } else {
        showbackground = 0;
    }
    return showbackground;
}

+ (bool) flipLearning {
    return flipLearningRate();
}

+ (double) getGameScoreObjC {
    return getGameScore();
}

+ (int) getNumRepsCompletedTotObjC {
    return getNumRepsCompletedTot();
}

+ (void) resetCalibrationBackgroundObjC {
    /*NSURL *url = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"windshield_wiper" ofType:@"mp3"]];
    AVAudioPlayer *audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
    [[AVAudioSession sharedInstance] setActive: YES error: nil];
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    [audioPlayer play];*/

    resetCalibrationBackground();
}

+ (int) doSkipObjC {
    return doSkip();
}

+ (void) setDisplayExtraInfoObjC: (int)displayExtraInfoAmt {
    setDisplayExtraInfo(displayExtraInfoAmt);
}

+ (void) setGameSoundsEnabledObjC: (int)new_play_game_sounds {
    setGameSoundsEnabled(new_play_game_sounds);
}

+ (void) registerTapOfX: (double) x andY: (double) y andDisplayWidth: (NSInteger) displayWidth andDisplayHeight: (NSInteger) displayHeight {
    registerTap(x, y, (int) displayWidth, (int) displayHeight);
}

//Figured out the syntax for pass an image/other stuff as arguments through here from this link: https://stackoverflow.com/questions/30990512/using-opencv-with-swift. This post gave a link to an example project called OpenCVSwiftSwitch
//the plus (as opposed to minus) means static method
+ (UIImage*) processImageWithOpenCV: (UIImage*) img andIsTesting: (bool) isTesting andDoInteractiveTutorial: (bool) doInteractiveTutorial
{
    //NSArray* imageArray = [NSArray arrayWithObject:inputImage];
    //UIImage* result = [[self class] processWithArray:imageArray];
    //return result;

    //let stitchedImage:UIImage = CVWrapper.process(with: imageArray as! [UIImage]) as UIImage

    //UIImage *tplImg = [UIImage imageNamed:@"item1"];

    /*cv::Mat matImage;
     UIImageToMat(inputImage, matImage);
     UIImage* outputImage = MatToUIImage(img:processImage(matImage));*/

    //showbackground = 2;

    bool makeSmaller = false;

    cv::Mat matImageInput, matMask;
    cv::Mat matImageOutput;
    UIImageToMat(img, matImageInput);

    if (makeSmaller) {
        cv::Mat matImageInputSmall;
        cv::Mat matImageOutputSmall;
        cv::resize(matImageInput,   matImageInputSmall, cv::Size(), 0.25, 0.25, cv::INTER_NEAREST);
        std::tie(matImageOutputSmall, matMask) = inputImage(matImageInputSmall, -1, -1, isTesting, doInteractiveTutorial);
        cv::resize(matImageOutputSmall, matImageOutput, cv::Size(),    4,    4, cv::INTER_NEAREST);
    } else {
        //cv::resize(matImageInput,   matImageInput, cv::Size(), 5, 5, cv::INTER_NEAREST);
        std::tie(matImageOutput, matMask) = inputImage(matImageInput, -1, -1,  isTesting, doInteractiveTutorial);
    }

    UIImage* outputImage = MatToUIImage(matImageOutput);
    return outputImage;

}

+ (UIImage*) processImageWithOpenCV: (UIImage*) img andIsTesting: (bool) isTesting andReturnWidth: (NSInteger) returnWidth andReturnHeight: (NSInteger) returnHeight andDoInteractiveTutorial: (bool) doInteractiveTutorial {

    cv::Mat matImageInput, matMask;
    cv::Mat matImageOutput;
    UIImageToMat(img, matImageInput);

    bool makeSmaller = false;
    if (makeSmaller) {
        cv::Mat matImageInputSmall;
        cv::Mat matImageOutputSmall;
        cv::resize(matImageInput,   matImageInputSmall, cv::Size(), 0.25, 0.25, cv::INTER_NEAREST);
        std::tie(matImageOutputSmall, matMask) = inputImage(matImageInputSmall, (int) returnWidth, (int) returnHeight, isTesting, doInteractiveTutorial);
        cv::resize(matImageOutputSmall, matImageOutput, cv::Size(),    4,    4, cv::INTER_NEAREST);

    } else {
        std::tie(matImageOutput, matMask) = inputImage(matImageInput, (int) returnWidth, (int) returnHeight, isTesting, doInteractiveTutorial);
    }


    UIImage* outputImage = MatToUIImage(matImageOutput);
    return outputImage;

}


+ (void) initializeLightsTutorialObjC:(NSString*) json_data_str {
    initializeLightsTutorial([json_data_str UTF8String]);
}
+ (bool) getLightsTutorialDoneStateObjC {
    return getLightsTutorialDoneState();
}


+ (UIImage*) doLightsTutorialObjC: (UIImage*) img andReturnWidth: (NSInteger) returnWidth andReturnHeight: (NSInteger) returnHeight {
    cv::Mat matImageInput;
    cv::Mat matImageOutput;
    UIImageToMat(img, matImageInput);
    
    matImageOutput = doLightsTutorial(matImageInput, (int) returnWidth, (int) returnHeight);
    
    UIImage* outputImage = MatToUIImage(matImageOutput);
    return outputImage;

}


//called every frame
+ (NSString*) getSoundToPlayIfAnyObjC {
    std::string cppOutput = getSoundToPlayIfAny();
    NSString* str = [NSString stringWithUTF8String:cppOutput.c_str()];
    return str;
    //alternatively, can make NSString directly here using literals like: return @"windshield_wiper"
}

@end
