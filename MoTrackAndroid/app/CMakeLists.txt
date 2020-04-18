## For more information about using CMake with Android Studio, read the
## documentation: https://d.android.com/studio/projects/add-native-code.html
## This file should be named "CMakeLists.txt" when being used



###--------------------------------------------------
###-------------------Set up CMAKE-------------------
###--------------------------------------------------
## Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")



###----------------------------------------------------
###------------Set up path to project------------------
###----------------------------------------------------
## Change path for your computer. This is the FIRST of the two places
## in this file where you have to do this. Below are the paths for Arik,
## Adam, Rahul, and Ben. Comment out all of the ones that are not for you.

## ARIK SHURYGEN- have this next line uncommented
#set(pathToProject C:/Users/Arik/AndroidStudioProjects/motracktest/MoTrackTherapyMobile/MoTrackAndroid)

## ADAM POLEVOY- have this next line uncommented
#set(pathToProject C:/Users/adamp/Documents/MoTrackAppCamera2/MoTrack)

## RAHUL YERRABELLI- have this next line uncommented
set(pathToProject /Users/ryerrabelli/AndroidStudioProjects/MoTrackTherapyMobile/MoTrackAndroid)

## BENJAMIN PIKUS- have this next line uncommented
#set(pathToProject C:/Users/bpiku/StudioProjects/MoTrackAndroid)

## BENJAMIN PIKUS (linux)- have this next line uncommented
#set(pathToProject /home/benka/StudioProjects/MoTrackTherapyMobile/MoTrackAndroid)



###-----------------------------------------------------------
###------------Add cpp files that we make ourselves-----------
###-----------------------------------------------------------
# Below code creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( native-lib SHARED src/main/cpp/native-lib.cpp )
add_library( calibration SHARED src/main/cpp/calibration.cpp )
add_library( motion_info SHARED src/main/cpp/motion_info.cpp )
add_library( audio_player SHARED src/main/cpp/audio_player.cpp )
add_library( bilinear_approx SHARED src/main/cpp/bilinear_approx.cpp )
add_library( fist_analysis SHARED src/main/cpp/fist_analysis.cpp )
add_library( hook_fist_analysis SHARED src/main/cpp/hook_fist_analysis.cpp )
add_library( pronation_analysis SHARED src/main/cpp/pronation_analysis.cpp )
add_library( deviation_windshield SHARED src/main/cpp/deviation_windshield.cpp )
add_library( fabduction_analysis SHARED src/main/cpp/fabduction_analysis.cpp )
add_library( topposition_analysis SHARED src/main/cpp/topposition_analysis.cpp )
add_library( deviation_catapult SHARED src/main/cpp/deviation_catapult.cpp )
add_library( flexion_knock SHARED src/main/cpp/flexion_knock.cpp )
add_library( flexion_cliff SHARED src/main/cpp/flexion_cliff.cpp )
add_library( pronation_dial SHARED src/main/cpp/pronation_dial.cpp )
add_library( pronation_ketchup SHARED src/main/cpp/pronation_ketchup.cpp )
add_library( fabduction_peacock SHARED src/main/cpp/fabduction_peacock.cpp )
add_library( topposition_alligator SHARED src/main/cpp/topposition_alligator.cpp )
add_library( fist_paddle SHARED src/main/cpp/fist_paddle.cpp )
add_library( hook_fist_caterpillar SHARED src/main/cpp/hook_fist_caterpillar.cpp )
add_library( interactive_tutorial SHARED src/main/cpp/interactive_tutorial.cpp )

###-----------------------------------------------------------
###------------Attach Preprocessor Directive Macros-----------
###-----------------------------------------------------------
# Use preprocesser directives to define the value of macro MY_OS to
# be +1, which represents Android. This tells the c++ to use the
# Android, not iOS, version of the c++ code.
add_definitions(-DMY_OS=1)



###------------------------------------------------------------
###-------------------Set OpenCv Library Path------------------
###------------------------------------------------------------
## Change path for your computer. This is the SECOND of the two places
## in this file where you have to do this. Below are the paths for Arik,
## Adam, Rahul, and Ben. Comment out all of the ones that are not for you.

## ARIK SHURYGEN- have this next line uncommented
#set(pathToOpenCv C:/Users/Arik/AndroidStudioProjects/motracktest/MoTrackTherapyMobile/MoTrackAndroid/OpenCV-android-sdk)

## ADAM POLEVOY- have this next line uncommented
#set(pathToOpenCv C:/Users/adamp/Documents/MoTrackAppCamera2/OpenCV-android-sdk)

## RAHUL YERRABELLI- have this next line uncommented
#OLD -set(pathToOpenCv /Users/ryerrabelli/NetBeansProjects/LeapMedProjects/Libraries/OpenCV-android-sdk)
#OLD - before Aug 8 2019 - set(pathToOpenCv /Users/ryerrabelli/AndroidStudioProjects/OpenCV-android-sdk_v3_1_0 )
set(pathToOpenCv /Users/ryerrabelli/AndroidStudioProjects/OpenCV-android-sdk_v4_1_1 )

## BENJAMIN PIKUS- have this next line uncommented
#set(pathToOpenCv C:/Users/bpiku/StudioProjects/OpenCV-android-sdk)

## BENJAMIN PIKUS (linux)- have this next line uncommented
#set(pathToOpenCv /home/benka/StudioProjects/OpenCV-android-sdk)



###-----------------------------------------------------------
###-------------------Add the OpenCv library------------------
###-----------------------------------------------------------
include_directories(${pathToOpenCv}/sdk/native/jni/include)
add_library( lib_opencv SHARED IMPORTED )
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${pathToProject}/app/src/main/jniLibs/${ANDROID_ABI}/libopencv_java4.so) #change path for your computer



###--------------------------------------------------------
###-------------------Add the log library------------------
###--------------------------------------------------------
## Searches for a specified prebuilt library and stores the path as a
## variable. Because CMake includes system libraries in the search path by
## default, you only need to specify the name of the public NDK library
## you want to add. CMake verifies that the library exists before
## completing its build.

find_library( log-lib log )



###-------------------------------------------------------
###-------------------Link the libraries------------------
###-------------------------------------------------------
## Specifies libraries CMake should link to your target library. You
## can link multiple libraries, such as libraries you define in this
## build script, prebuilt third-party libraries, or system libraries.

## The syntax of target_link_libraries is the first file links to all the other files. For example
## target_link_libraries( file1 file2 file3 file 4... means that file1 is linked to file2 and file 2 is
## linked to file 3, etc.

# Add all games into this one target_link_libraries function
target_link_libraries(  native-lib
        motion_info
        audio_player
        fist_analysis
        hook_fist_analysis
        bilinear_approx
        pronation_analysis
        fabduction_analysis
        topposition_analysis
        deviation_windshield
        deviation_catapult
		fist_paddle
        flexion_knock
        flexion_cliff
        pronation_dial
        pronation_ketchup
        fabduction_peacock
        topposition_alligator
        hook_fist_caterpillar
        interactive_tutorial
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

# Need to have one of this for each game, including plain games in analysis files (a whole new target_link_libraries function for each game)
target_link_libraries( bilinear_approx
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( fist_analysis
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( hook_fist_analysis
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( pronation_analysis
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( fabduction_analysis
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( topposition_analysis
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( deviation_windshield
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( deviation_catapult
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( flexion_knock
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( flexion_cliff
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( pronation_dial
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( pronation_ketchup
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( fabduction_peacock
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( fist_paddle
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( topposition_alligator
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( hook_fist_caterpillar
        calibration
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( interactive_tutorial
        audio_player
        calibration
		fist_analysis
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

#Used to connect a game to its general analysis function
target_link_libraries( flexion_knock
        bilinear_approx
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( flexion_cliff
        bilinear_approx
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( deviation_windshield
        bilinear_approx
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( deviation_catapult
        bilinear_approx
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( pronation_dial
        pronation_analysis
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( pronation_ketchup
        pronation_analysis
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( fabduction_peacock
        fabduction_analysis
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( topposition_alligator
        topposition_analysis
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( hook_fist_caterpillar
        hook_fist_analysis
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

# Need to link calibration and sound, doesn't have to be changed for each game
target_link_libraries( calibration
        motion_info
        -ljnigraphics
        -landroid
        ${log-lib}
        lib_opencv)

target_link_libraries( audio_player
        -ljnigraphics
        -landroid
        ${log-lib}
        OpenSLES)



###--------------------------------------------------
###-------------------Eigen library------------------
###--------------------------------------------------
## Proof of concept that we can get header libraries in (Eigen is a lin alg library)
include_directories(${pathToProject}/app/src/main/cpp/Eigen)

