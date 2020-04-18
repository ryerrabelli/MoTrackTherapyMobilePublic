//
// Created by bpiku on 12/18/2018.
//  MoTrackTestiOS/MoTrack Android
//
//  Created by Rahul Yerrabelli on 12/27/18 / 11/24/18
//  Copyright © 2018 MoTrack Therapy. All rights reserved.
//

//The calibration import must always be the first import in order to make use of device-dependent preprocessor directives
#include "calibration.h"
//#include "all_access_vars.h"


//device specific includes/imports
#if MY_OS==ANDROID_OS
    //Below are the imports for the android version
    #include <stdlib.h>
    #include <jni.h>
    #include <pthread.h>

    // for native audio
    #include <SLES/OpenSLES.h>
    #include <SLES/OpenSLES_Android.h>

    // for native asset manager
    #include <sys/types.h>
    #include <android/asset_manager.h>
    #include <android/asset_manager_jni.h>

#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    //Below are the imports for the iOS version
    #include <stdio.h>


#endif


// Below are all the non-device-specific imports, starting with the standard libraries in the beginning
// (surrounded by < >) and then the libraries we designed (surrounded by quotes)
#include <string.h>

#include "audio_player.h"


using namespace cv;
using namespace std;

#ifdef EXTERN_C
extern "C" {
#endif
    
    
int play_game_sounds = 1;

    
#if MY_OS==ANDROID_OS
/*
 * ------------------------------------------------------------------------------------------------
 *                                  JAVA TO CPP INTERFACE FUNCTIONS & VARIABLES
 * ------------------------------------------------------------------------------------------------
 */
extern "C" {

// Variables
// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
// file descriptor player interfaces
static SLObjectItf fdPlayerObject = NULL;
static SLPlayItf fdPlayerPlay_CorrectSound;
static SLPlayItf fdPlayerPlay_WindshieldSound;
static SLPlayItf fdPlayerPlay_DoorknockSound;
static SLPlayItf fdPlayerPlay_SqueezeSound;
static SLPlayItf fdPlayerPlay_SkipSound;
static SLPlayItf fdPlayerPlay_BalloonPopSound;
static SLPlayItf fdPlayerPlay_CatapultBreakSound;
static SLPlayItf fdPlayerPlay_CatapultThrowSound;
static SLPlayItf fdPlayerPlay_CliffCollapseSound;
static SLPlayItf fdPlayerPlay_NomNomNomSound;
    
static SLSeekItf fdPlayerSeek_Correct;
static SLSeekItf fdPlayerSeek_Doorknock;
static SLSeekItf fdPlayerSeek_Windshield;
static SLSeekItf fdPlayerSeek_Squeeze;
static SLSeekItf fdPlayerSeek_Skip;
static SLSeekItf fdPlayerSeek_BalloonPop;
static SLSeekItf fdPlayerSeek_CatapultBreak;
static SLSeekItf fdPlayerSeek_CatapultThrow;
static SLSeekItf fdPlayerSeek_CliffCollapse;
static SLSeekItf fdPlayerSeek_NomNomNom;

static SLMuteSoloItf fdPlayerMuteSolo;
static SLVolumeItf fdPlayerVolume;
static AAssetManager* mgr;
// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


void Java_com_motracktherapy_motrack_CVActivity_setGameSoundsEnabled(JNIEnv *env, jobject, jint new_play_game_sounds) {
    setGameSoundsEnabled( (int) new_play_game_sounds );
}
    
// create the engine and output mix objects
void Java_com_motracktherapy_motrack_CVActivity_createEngine(JNIEnv* env, jclass clazz) {
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    //assert(SL_RESULT_SUCCESS == result);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    (void)result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    //assert(SL_RESULT_SUCCESS == result);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    (void)result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example

}

// create asset audio player
jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayer(JNIEnv* env, jclass clazz,
                                                                         jobject assetManager, jstring filenameCorrect,
                                                   jstring filenameWindshield, jstring filenameDoorknock, jstring filenameSqueeze,
                                                                           jstring filenameSkip) {
    SLresult result;

    // convert Java string to UTF-8

    const char *utf8Correct = env->GetStringUTFChars( filenameCorrect, NULL);
    const char *utf8Windshield = env->GetStringUTFChars( filenameWindshield, NULL);
    const char *utf8Doorknock = env->GetStringUTFChars( filenameDoorknock, NULL);
    const char *utf8Squeeze = env->GetStringUTFChars( filenameSqueeze, NULL);
    const char *utf8Skip = env->GetStringUTFChars( filenameSkip, NULL);
    if (NULL == utf8Correct || NULL == utf8Windshield || NULL == utf8Doorknock || NULL == utf8Squeeze
            || NULL== utf8Skip ) {
        return JNI_FALSE;
    }
    //assert(NULL != utf8);

    // use asset manager to open asset by filename
    mgr = AAssetManager_fromJava(env, assetManager);
    if (NULL == mgr) {
        return JNI_FALSE;
    }
    jboolean resCorrect =  createSinglePlayer(mgr,utf8Correct, fdPlayerPlay_CorrectSound,fdPlayerSeek_Correct, false );
    jboolean resDoorknock =  createSinglePlayer(mgr,utf8Doorknock, fdPlayerPlay_DoorknockSound, fdPlayerSeek_Doorknock, false);
    jboolean resWindshield =  createSinglePlayer(mgr,utf8Windshield, fdPlayerPlay_WindshieldSound,fdPlayerSeek_Windshield, true);
    jboolean resSqueeze =  createSinglePlayer(mgr,utf8Squeeze, fdPlayerPlay_SqueezeSound,fdPlayerSeek_Squeeze, false);
    jboolean resSkip =  createSinglePlayer(mgr,utf8Skip, fdPlayerPlay_SkipSound,fdPlayerSeek_Skip, false);
    // release the Java string and UTF-8
    env->ReleaseStringUTFChars( filenameCorrect, utf8Correct);
    env->ReleaseStringUTFChars( filenameDoorknock, utf8Doorknock);
    env->ReleaseStringUTFChars( filenameWindshield, utf8Windshield);
    env->ReleaseStringUTFChars( filenameSqueeze, utf8Squeeze);
    env->ReleaseStringUTFChars( filenameSkip, utf8Skip);
    return (resSkip && resCorrect && resDoorknock && resWindshield && resSqueeze) ;
}


// create asset audio player
jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerBalloonGame(JNIEnv* env, jclass clazz,
                                                                           jobject assetManager, jstring filenameBalloon) {
    SLresult result;

    // convert Java string to UTF-8
    const char *utf8Balloon = env->GetStringUTFChars( filenameBalloon, NULL);
    if (NULL == utf8Balloon ) {
        return JNI_FALSE;
    }
    //assert(NULL != utf8);

    // use asset manager to open asset by filename
    mgr = AAssetManager_fromJava(env, assetManager);
    if (NULL == mgr) {
        return JNI_FALSE;
    }
    jboolean resBalloon =  createSinglePlayer(mgr,utf8Balloon, fdPlayerPlay_BalloonPopSound,fdPlayerSeek_BalloonPop, false );
    // release the Java string and UTF-8
    env->ReleaseStringUTFChars( filenameBalloon, utf8Balloon);
    return resBalloon;
}

// create asset audio player
jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerCatapultGame(JNIEnv* env, jclass clazz,
                                                                                      jobject assetManager, jstring filenameCatapultBreak, jstring filenameCatapultThrow) {
    SLresult result;

    // convert Java string to UTF-8
    const char *utf8CatapultBreak = env->GetStringUTFChars( filenameCatapultBreak, NULL);
    const char *utf8CatapultThrow = env->GetStringUTFChars( filenameCatapultThrow, NULL);
    if (NULL == utf8CatapultBreak || NULL == utf8CatapultThrow ) {
        return JNI_FALSE;
    }
    //assert(NULL != utf8);

    // use asset manager to open asset by filename
    mgr = AAssetManager_fromJava(env, assetManager);
    if (NULL == mgr) {
        return JNI_FALSE;
    }
    jboolean resBreak=  createSinglePlayer(mgr,utf8CatapultBreak, fdPlayerPlay_CatapultBreakSound,fdPlayerSeek_CatapultBreak, false );
    jboolean resThrow=  createSinglePlayer(mgr,utf8CatapultThrow, fdPlayerPlay_CatapultThrowSound,fdPlayerSeek_CatapultThrow, false );
    // release the Java string and UTF-8
    env->ReleaseStringUTFChars( filenameCatapultBreak, utf8CatapultBreak);
    env->ReleaseStringUTFChars( filenameCatapultThrow, utf8CatapultThrow);
    return resBreak && resThrow;
}

// create asset audio player
jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerCliffGame(JNIEnv* env, jclass clazz,
                                                                                       jobject assetManager, jstring filenameCliffCollapse) {
    SLresult result;

    // convert Java string to UTF-8
    const char *utf8CCliffCollapse = env->GetStringUTFChars( filenameCliffCollapse, NULL);
    if (NULL == utf8CCliffCollapse) {
        return JNI_FALSE;
    }
    //assert(NULL != utf8);

    // use asset manager to open asset by filename
    mgr = AAssetManager_fromJava(env, assetManager);
    if (NULL == mgr) {
        return JNI_FALSE;
    }
    jboolean resCollapse=  createSinglePlayer(mgr,utf8CCliffCollapse, fdPlayerPlay_CliffCollapseSound,fdPlayerSeek_CliffCollapse, false );
    // release the Java string and UTF-8
    env->ReleaseStringUTFChars( filenameCliffCollapse, utf8CCliffCollapse);
    return resCollapse;
}
    
// create asset audio player
jboolean Java_com_motracktherapy_motrack_CVActivity_createAssetAudioPlayerKetchupGame(JNIEnv* env, jclass clazz,
                                                                                    jobject assetManager, jstring filenameNomNomNom) {
    SLresult result;
    
    // convert Java string to UTF-8
    const char *utf8CNomNomNom = env->GetStringUTFChars( filenameNomNomNom, NULL);
    if (NULL == utf8CNomNomNom) {
        return JNI_FALSE;
    }
    //assert(NULL != utf8);
    
    // use asset manager to open asset by filename
    mgr = AAssetManager_fromJava(env, assetManager);
    if (NULL == mgr) {
        return JNI_FALSE;
    }
    jboolean resCollapse=  createSinglePlayer(mgr,utf8CNomNomNom, fdPlayerPlay_NomNomNomSound,fdPlayerSeek_NomNomNom, false );
    // release the Java string and UTF-8
    env->ReleaseStringUTFChars( filenameNomNomNom, utf8CNomNomNom);
    return resCollapse;
}
    
// create asset audio player
jboolean createSinglePlayer(AAssetManager* mgr, const char *utf8, SLPlayItf& fdPlayer, SLSeekItf& fdPlayerSeek, bool shouldLoop ) {
    SLresult result;

    //assert(NULL != mgr);
    AAsset *asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);

    // the asset might not be found
    if (NULL == asset) {
        return JNI_FALSE;
    }

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    //assert(0 <= fd);
    if (0 > fd) {
        return JNI_FALSE;
    }
    AAsset_close(asset);

    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayerObject, &audioSrc, &audioSnk,
                                                3, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }
    //assert(SL_RESULT_SUCCESS == result);

    (void) result;

    // realize the player
    result = (*fdPlayerObject)->Realize(fdPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the play interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_PLAY, &fdPlayer);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the seek interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_SEEK, &fdPlayerSeek);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the mute/solo interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_MUTESOLO, &fdPlayerMuteSolo);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the volume interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_VOLUME, &fdPlayerVolume);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }
    //assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // enable whole file looping
    if (shouldLoop) {
        result = (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);

        if (SL_RESULT_SUCCESS != result) {
            return JNI_FALSE;
        }
        //assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }
    return JNI_TRUE;
}

// shut down the native audio system
void  Java_com_motracktherapy_motrack_CVActivity_shutdownAudio(JNIEnv* env, jclass clazz) {

    // destroy file descriptor audio player object, and invalidate all associated interfaces
    if (fdPlayerObject != NULL) {
        (*fdPlayerObject)->Destroy(fdPlayerObject);
        fdPlayerObject = NULL;
        fdPlayerPlay_CorrectSound =NULL;
        fdPlayerPlay_BalloonPopSound =NULL;
        fdPlayerPlay_WindshieldSound=NULL;
        fdPlayerPlay_DoorknockSound= NULL;
        fdPlayerPlay_SqueezeSound= NULL;
        fdPlayerSeek_Correct= NULL;
        fdPlayerSeek_Doorknock= NULL;
        fdPlayerSeek_Windshield= NULL;
        fdPlayerSeek_Squeeze= NULL;
        fdPlayerMuteSolo = NULL;
        fdPlayerVolume = NULL;
    }


    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

}

//Below two methods aren't java_com_... methods, but they still can only be called from Android because they require libraries imported only in java
// if shouldPlayOnLoop is true, keep playing loop. if false, stop playing loop
void playAudioFileLoop(SLPlayItf fdPlayerPlay, SLSeekItf fdPlayerSeek, bool shouldPlayOnLoop) {
    if (play_game_sounds == GAME_SOUNDS_OFF) return;
    SLresult result;
    // make sure the asset audio player was created
    if (NULL != fdPlayerPlay) {

        // set the player's state
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, shouldPlayOnLoop ?  SL_PLAYSTATE_PLAYING:SL_PLAYSTATE_PAUSED);

        (void) result;
    }

}

void playAudioFile(SLPlayItf& fdPlayerPlay, SLSeekItf& fdPlayerSeek) {
    if (play_game_sounds == GAME_SOUNDS_OFF) return;

    
    SLresult result;
    // make sure the asset audio player was created
    if (NULL != fdPlayerPlay) {
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, SL_PLAYSTATE_STOPPED);
        //result = (*fdPlayerVolume)->SetMute(fdPlayerVolume, SL_BOOLEAN_FALSE);
        //SLuint32 state;
        //result = (*fdPlayerPlay)->GetPlayState(fdPlayerPlay, &state);
        //SLmillisecond msec;
        /*result = (*fdPlayerPlay)->GetPosition(fdPlayerPlay, &msec);
         (void) result;
         result = (*fdPlayerSeek)->SetPosition(fdPlayerSeek, 0, SL_SEEKMODE_FAST);
         (void) result;

         result = (*fdPlayerPlay)->GetPosition(fdPlayerPlay, &msec);
         (void) result;*/
        //result = (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
        // set the player's state
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, SL_PLAYSTATE_PLAYING);
        //result = (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
        //result = (*fdPlayerPlay)->GetPlayState(fdPlayerPlay, &state);
        //assert(SL_RESULT_SUCCESS == result);
        /*(void) result;
         result = (*fdPlayerPlay)->GetPosition(fdPlayerPlay, &msec);
         while (msec < 1971) {
         result = (*fdPlayerPlay)->GetPosition(fdPlayerPlay, &msec);
         }
         result = (*fdPlayerVolume)->SetMute(fdPlayerVolume, SL_BOOLEAN_TRUE);*/
    }

}

} //end of extern "C"


#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
/*
 * ---------------------------------------------------------------------------
 *                         iOS INTERFACE FUNCTIONS
 * ---------------------------------------------------------------------------
 */


std::string soundNameToPlay;

std::string getSoundToPlayIfAny() {
    std::string toReturn = soundNameToPlay;
    soundNameToPlay = ""; //don't make it keep starting the sound every frame
    return (play_game_sounds == GAME_SOUNDS_OFF) ? "" : toReturn;
}

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
    
void setGameSoundsEnabled(int new_play_game_sounds) {
    play_game_sounds = new_play_game_sounds;
}
    
//bool prevPlayStatusCorrect = false;
void playAudioFileCorrect() {
    #if MY_OS==ANDROID_OS
        playAudioFile(fdPlayerPlay_CorrectSound, fdPlayerSeek_Correct);
    #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
        soundNameToPlay = "correct.mp3";
    #endif
}

void playAudioFileClick() {
    #if MY_OS==ANDROID_OS
        playAudioFile(fdPlayerPlay_SkipSound, fdPlayerSeek_Skip);
    #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
        soundNameToPlay = "click.wav";
    #endif
}

// will play the audio on loop if should play is true (Android only; in iOS, plays only once)
void playAudioFileWindshield(bool shouldPlayOnLoop) {
    #if MY_OS==ANDROID_OS
        playAudioFileLoop(fdPlayerPlay_WindshieldSound, fdPlayerSeek_Windshield, shouldPlayOnLoop);
    #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
        soundNameToPlay = "windshield_wiper.mp3";
    #endif
}

void playAudioFileDoorknock() {
    #if MY_OS==ANDROID_OS
        playAudioFile(fdPlayerPlay_DoorknockSound, fdPlayerSeek_Doorknock);
    #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
        soundNameToPlay = "door_knock.mp3";
    #endif
}

void playAudioFileSqueeze() {
    #if MY_OS==ANDROID_OS
        playAudioFile(fdPlayerPlay_SqueezeSound, fdPlayerSeek_Squeeze);
    #elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
        soundNameToPlay = "squeeze.mp3";
    #endif
}

void playAudioFileBalloonPop() {
#if MY_OS==ANDROID_OS
    playAudioFile(fdPlayerPlay_BalloonPopSound, fdPlayerSeek_BalloonPop);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "balloon_pop_sound.mp3";
#endif
}

void playAudioFileCatapultBreak() {
#if MY_OS==ANDROID_OS
    playAudioFile(fdPlayerPlay_CatapultBreakSound, fdPlayerSeek_CatapultBreak);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "catapult_break.mp3";
#endif

}

void playAudioFileCatapultThrow() {
#if MY_OS==ANDROID_OS
    playAudioFile(fdPlayerPlay_CatapultThrowSound, fdPlayerSeek_CatapultThrow);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "catapult_throw.mp3";
#endif
}

void playAudioFileCliffCollapses() {
#if MY_OS==ANDROID_OS
     playAudioFile(fdPlayerPlay_CliffCollapseSound, fdPlayerSeek_CliffCollapse);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "cliff_collapse.mp3";
#endif
}
    
void playAudioFileEating() {
#if MY_OS==ANDROID_OS
    playAudioFile(fdPlayerPlay_NomNomNomSound, fdPlayerSeek_NomNomNom);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "nomnomnom.mp3";
#endif
}
    
void playAudioFileSpill() {
#if MY_OS==ANDROID_OS
    //TEMPORARY UNTIL BEN IMPLEMENTS ANDROID VERSION
    //playAudioFile(fdPlayerPlay_CorrectSound, fdPlayerSeek_Correct);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "small_water_spill.mp3";
#endif
}
    
void playAudioFileCamera() {
#if MY_OS==ANDROID_OS
    //TEMPORARY UNTIL BEN IMPLEMENTS ANDROID VERSION
    //playAudioFile(fdPlayerPlay_CorrectSound, fdPlayerSeek_Correct);
#elif (MY_OS==IOS || MY_OS==MAC_OS || MY_OS==LINUX_OS)
    soundNameToPlay = "camera_sound.wav";
#endif
}
    

#ifdef EXTERN_C
}
#endif

