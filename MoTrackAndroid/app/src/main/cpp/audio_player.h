//
// Created by bpiku on 12/20/2018.
//

#ifndef MOTRACKTHERAPYMOBILE_PLAYAUDIO_H
#define MOTRACKTHERAPYMOBILE_PLAYAUDIO_H

#ifdef EXTERN_C
extern "C" {
#endif

    
#define GAME_SOUNDS_OFF 0
#define GAME_SOUNDS_ON 0
    
    
#if MY_OS==ANDROID_OS

#include <android/asset_manager.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <jni.h>
    
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    //Below are the imports for the iOS version
#include <stdio.h>
    
#endif

    extern int play_game_sounds;

    
#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS
 * ------------------------------------------------------------------------------------------------
 */
extern "C" {

void Java_com_motracktherapy_motrack_CVActivity_setGameSoundsEnabled(JNIEnv *env, jobject, jint new_play_game_sounds);
    
void Java_com_motracktherapy_motrack_CVActivity_createEngine(JNIEnv* env, jclass clazz);

jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayer(JNIEnv* env, jclass clazz,
                                                                           jobject assetManager, jstring filenameCorrect,
                                                                           jstring filenameWindshield, jstring filenameDoorknock, jstring filenameSqueeze, jstring filenameSkip);

jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerBalloonGame(JNIEnv* env, jclass clazz,
                                                                                      jobject assetManager, jstring filenameBalloon);

jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerCatapultGame(JNIEnv* env, jclass clazz,
                                                                                       jobject assetManager, jstring filenameCatapultBreak, jstring filenameCatapultThrow);
jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerCliffGame(JNIEnv* env, jclass clazz,
                                                                                       jobject assetManager, jstring filenameCliffCollapse);

jboolean createSinglePlayer(AAssetManager* mgr, const char *utf8, SLPlayItf& fdPlayer, SLSeekItf& fdPlayerSeek, bool );
    
void  Java_com_motracktherapy_motrack_CVActivity_shutdownAudio(JNIEnv* env, jclass clazz);
    
void playAudioFileLoop(SLPlayItf fdPlayerPlay, SLSeekItf fdPlayerSeek, bool shouldPlayOnLoop);
    
void playAudioFile(SLPlayItf& fdPlayerPlay, SLSeekItf& fdPlayerSeek);

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */
    

std::string getSoundToPlayIfAny();

#endif
/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *                                  END OF INTERFACE FUNCTIONS
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
    
/*
 * ------------------------------------------------------------------------------------------------
 *                                         SOUND CORE
 *                 Functions called by the rest of the program to play sound
 * ------------------------------------------------------------------------------------------------
 */

void setGameSoundsEnabled(int new_play_game_sounds);

void playAudioFileCorrect();
void playAudioFileClick();
void playAudioFileWindshield(bool shouldPlay);
void playAudioFileDoorknock();
void playAudioFileSqueeze();
void playAudioFileBalloonPop();
void playAudioFileCatapultBreak();
void playAudioFileCatapultThrow();
void playAudioFileCliffCollapses();
void playAudioFileEating();
void playAudioFileSpill();
void playAudioFileCamera();

#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_PLAYAUDIO_H
