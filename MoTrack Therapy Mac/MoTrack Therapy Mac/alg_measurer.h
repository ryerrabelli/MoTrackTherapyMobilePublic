//
//  alg_measurer.cpp
//  MoTrack Therapy Mac
//
//  Created by Rahul Yerrabelli on 6/21/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef alg_measurer_h
#define alg_measurer_h

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "json.hpp"


using namespace std;
using namespace cv;
using namespace nlohmann;


json measureAlgorithmPerformance(vector<int> setNums, int setNameLen, int segmentationAlg, json data, bool shouldSkipIfRedboxUnsolvable, bool shouldSave, string saveSuffix);

void testAlgorithmMeasurement();

json analyzeMeasurementData(json data, vector<int> setNums, int setNameLen);


#endif /* alg_measurer_h */
