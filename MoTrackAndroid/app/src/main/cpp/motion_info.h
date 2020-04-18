//
// Created by benka on 4/19/2018.
//

#ifndef MOTRACKTHERAPYMOBILE_MOTION_INFO_H
#define MOTRACKTHERAPYMOBILE_MOTION_INFO_H

#include <map>
#include <string>


#define COMPARE_GREATER_THAN +1 //the param for the frame has to be greater than the threshold
#define COMPARE_LESS_THAN -1 //the param for the frame has to be less than the threshold
#define COMPARE_FURTHER_AWAY_FROM_0 0 //the param for the frame has to be further away from 0 than the threshold (default)


#ifdef EXTERN_C
extern "C" {
#endif

// This holds the information for each individual motion
struct MotionInfo{
    int numRepsCompleted; //nrc
    int numRepsToBeCompleted; //nrtc
    int numSkips; //ns
    std::string motionInstructions; //mInst
    float param; // param necessary to "complete" motion, will probably change this
    int comparisonMethod;
    //std::string motionCompleted(); // analyzes to determine if motion is completed, for now takes nothing
};

// This holds the map that stores all the active motions
struct MotionsMap {
    std::map<int,MotionInfo> motions;
    // Below funcs returns motions (the map). Funcs are the same except first creates the MotionInfo from input
    void addMotion(int,int,int,int,std::string, float, int);
    void addMotion(int,int,int,int,std::string, float);
    void addMotion(int,MotionInfo);
    int numRepsCompletedTot; // ntrc_tot
    int numRepsToBeCompletedTot; // nrtbc_tot
    int numSkipsTot; // ns_tot, not useful/necessary variable
};

// Creates and returns an empty MotionsMap struct which holds the map of active motions
MotionsMap createMap();

// Creates and returns an empty Info struct which holds the information for each individual motion
// Order of parameters: numRepsCompleted, numRepsToBeCompleted, numSkips;
MotionInfo createMotionInfo(int, int, int, std::string, float, int);

    
#ifdef EXTERN_C
}
#endif

#endif //MOTRACKTHERAPYMOBILE_MOTION_INFO_H
