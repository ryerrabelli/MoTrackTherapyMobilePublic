//
//  OpenCVWrapper.h
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 12/25/18.
//  Recreated by Rahul Yerrabelli on 2/13/19.
//  Copyright © 2018/2019 MoTrack Therapy. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <UIKit/UIKit.h>


NS_ASSUME_NONNULL_BEGIN

@interface OpenCVWrapper : NSObject
- (void)isThisWorking;

- (void)isThisWorkingNum:(int)mynum;

+ (int) getDoneStateObjC;

+ (NSString*) getCrashErrorDescriptionObjC;
+ (NSString*) getCrashErrorDescriptionCodedObjC;

+ (void) playAudioFileClickObjC;
+ (void) playAudioFileCorrectObjC;
+ (void) playAudioFileDoorKnockbjC;
+ (void) playAudioFileEatingObjC;
+ (void) playAudioFileCameraObjC;

+ (void) setupImagesFor:(int)exercise_game;

//ObjC's NSInteger gets wrapped to swift's int. ObjC's int gets wrapped to swift's int32. See https://stackoverflow.com/questions/28102927/cannot-call-an-objc-method-taking-int-in-swift-because-int32-cannot-be-converted
+ (void) initializeObjcWithReps:(NSInteger)nrtbc_each andHand:(NSInteger)hand andJSONDataStr:(NSString*) json_data_str;

+ (int) getOrientationObjC:(int)exercise_game;

+ (bool) shouldCameraBeInitializedAsContinuousObjC;

+ (void) setExerciseGame:(int)exercise_game andOrientationGame:(int) orientation_game;

+ (void) setShowBackground:(int)n_showbackground;

+ (int) incrementShowBackground;

+ (bool) flipLearning;

+ (double) getGameScoreObjC;

+ (int) getNumRepsCompletedTotObjC;

+ (NSString*) getSoundToPlayIfAnyObjC; //called every frame

+ (void) resetCalibrationBackgroundObjC;

+ (int) doSkipObjC;

+ (void) registerTapOfX: (double) x andY: (double) y andDisplayWidth: (NSInteger) imageDisplay andDisplayHeight: (NSInteger) imageDisplay;

+ (UIImage*) processImageWithOpenCV: (UIImage*) img andIsTesting: (bool) isTesting andDoInteractiveTutorial: (bool) doInteractiveTutorial;

+ (UIImage*) processImageWithOpenCV: (UIImage*) img andIsTesting: (bool) isTesting andReturnWidth: (NSInteger) returnWidth andReturnHeight: (NSInteger) returnHeight andDoInteractiveTutorial: (bool) doInteractiveTutorial;

+ (void) initializeLightsTutorialObjC:(NSString*) json_data_str;
+ (bool) getLightsTutorialDoneStateObjC;
+ (UIImage*) doLightsTutorialObjC: (UIImage*) img andReturnWidth: (NSInteger) returnWidth andReturnHeight: (NSInteger) returnHeight;


+ (void) setDisplayExtraInfoObjC: (int)displayExtraInfoAmt;

+ (void) setGameSoundsEnabledObjC: (int)new_play_game_sounds;

@end

NS_ASSUME_NONNULL_END

