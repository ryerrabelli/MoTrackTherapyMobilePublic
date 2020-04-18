//
// Holds all the motion information for app
// Created by benka on 4/19/2018.
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
#include <map>

#include "motion_info.h"

#ifdef EXTERN_C
extern "C" {
#endif

MotionsMap createMap() {
    MotionsMap motionsMap;
    //If these values are not set, erratic behavior happens
    motionsMap.numRepsCompletedTot     = 0;
    motionsMap.numRepsToBeCompletedTot = 0;
    motionsMap.numSkipsTot             = 0;
    return motionsMap;
}

/*
 * nrc: number of reps completed, nrtbc: number of reps to be completed,
 * ns: number of skips, mInst: motion instructions, pa: param
 */
MotionInfo createMotionInfo(int nrc_each, int nrtbc_each, int ns_each, std::string mInst, float pa, int comparisonMethod) {
    MotionInfo newMotionInfo = {nrc_each, nrtbc_each, ns_each, mInst, pa, comparisonMethod}; // this way is slightly faster than the one below I think
    /*info.numRepsCompleted = nrc;
    info.numRepsToBeCompleted = nrtbc;
    info.numSkips = ns;*/
    return newMotionInfo;
}

void MotionsMap::addMotion(int ind, int nrc_each, int nrtbc_each, int ns_each, std::string mInst, float pa, int comparisonMethod) {
    MotionInfo newMotionInfo = createMotionInfo(nrc_each, nrtbc_each, ns_each, mInst, pa, comparisonMethod);
    MotionsMap::addMotion(ind, newMotionInfo);
}
void MotionsMap::addMotion(int ind, int nrc_each, int nrtbc_each, int ns_each, std::string mInst, float pa) {
    int comparisonMethod = 0;
    MotionInfo newMotionInfo = createMotionInfo(nrc_each, nrtbc_each, ns_each, mInst, pa, comparisonMethod);
    MotionsMap::addMotion(ind, newMotionInfo);
}
void MotionsMap::addMotion(int ind, MotionInfo motionInfo) {
    numRepsCompletedTot += motionInfo.numRepsCompleted;
    numRepsToBeCompletedTot += motionInfo.numRepsToBeCompleted;
    numSkipsTot += motionInfo.numSkips; //not useful/necessary variable
    MotionsMap::motions[ind] = motionInfo;
}

#ifdef EXTERN_C
}
#endif
