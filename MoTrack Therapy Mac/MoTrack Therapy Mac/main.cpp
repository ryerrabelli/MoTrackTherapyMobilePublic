//
//  main.cpp
//  MoTrack Therapy Mac
//
//  Created by Rahul Yerrabelli on March 5, 2019.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

//used this tutorial to get opencv for c++ working on mac (only used the parts I needed; didn't really have to do any of the beginning terminal steps): https://medium.com/@jaskaranvirdi/setting-up-opencv-and-c-development-environment-in-xcode-b6027728003

#include <iostream>
#include <opencv2/opencv.hpp>

#include "main.h"
#include "alg_measurer.h"
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


json getJSONFromFile(string fileName) {
    std::ifstream ifs(fileName);
    json data = json::parse(ifs);
    return data;
}

/*
 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 ------------------------------------------------------------------------------------------
 |                             THIS IS THE  *MAIN*  FUNCTION                                |
 ------------------------------------------------------------------------------------------
 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
int main(int argc, const char * argv[]) {
    if (false) {
        testAlgorithmMeasurement();
        return 0;
    }
    
    if (true) {
        playGame();
    }
    
    if (false) {
        vector<int> setNums = {};
        for (int i = 1; i <= 42; i++) setNums.push_back(i);
        const int setNameLen = 4; //number of characters in the string version of setNameLen
        int segmentationAlg = 1684;
        
        bool shouldSave = true;
        string saveSuffix = "_betterGraphicsSameOrder";
        bool shouldSkipIfRedboxUnsolvable = true; //segmentationAlg!=1684;

        json data;
        //data = getJSONFromFile("photos/photo_analysis/" + std::to_string(segmentationAlg) + saveSuffix + "/data_unanalyzed_"+std::to_string(segmentationAlg)+saveSuffix+".json");
        
        data = measureAlgorithmPerformance( setNums, setNameLen, segmentationAlg, data, shouldSkipIfRedboxUnsolvable, shouldSave, saveSuffix);
        
        
        data = analyzeMeasurementData(data, setNums, setNameLen);
        
        
        if (shouldSave) {
            std::string jsonAnalyzedFileName = "photos/photo_analysis/" + std::to_string(segmentationAlg) + saveSuffix + "/data_analyzed_"+std::to_string(segmentationAlg)+saveSuffix+".json";
            std::ofstream analyzedFile(jsonAnalyzedFileName);
            analyzedFile << data;
            cout << "Saved analyzed json data to file '" + jsonAnalyzedFileName + "'. " << endl;
            cout << endl;
        }
        
        return 0;
    }
    
}


