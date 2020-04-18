//
//  alg_measurer.cpp
//  MoTrack Therapy Mac
//
//  Created by Rahul Yerrabelli on 6/21/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include "json.hpp"

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


std::string makeStrFromInt(int num, int length, char leadChar) {
    if (num == 0) return std::string(length-1,leadChar) + "0";
    else return std::string( std::max(0,length-1-int(log10(num)+0.00001)) , leadChar) + std::to_string(num);
}

json measureAlgorithmPerformance(vector<int> setNums, int setNameLen, int segmentationAlg, json data, bool shouldSkipIfRedboxUnsolvable, bool shouldSave, std::string saveSuffix) {
    std::chrono::steady_clock::time_point measurementStartTime = std::chrono::steady_clock::now();
    std::string photoPath = "photos/real_photos/";
    
    int nrtbc_each = 4;
    Hand hand = Hand::RIGHT;
    int exerciseGame = FIST_PUMP_BALLOON_GAME;
    bool show = false;
    bool saveExtraInfo = true;
    
    vector< vector<float> > allIoUs; //IoU stands for intersection over union
    vector< vector<float> > allAccuracies;
    vector< vector<float> > allParamDiffs; //IoU stands for intersection over union
    data["runParameters"]["exerciseGame"] = exerciseGame;
    data["runParameters"]["segmentationAlgType"] = segmentationAlg;
    data["runParameters"]["segmentationAlg"] = std::to_string(segmentationAlg) + saveSuffix;
    data["runParameters"]["saveSuffix"] = saveSuffix;
    data["runParameters"]["hand"] = hand;
    data["runParameters"]["handEnum"] = hand==Hand::RIGHT ? "Right" : (hand==Hand::LEFT ? "Left" : "N/A");
    data["runParameters"]["cppVersion"] = CPPVERSION;
    
    auto setNumsSize = setNums.size(); //supposedly, separating this out is better for speed
    for (int it = 0; it < setNumsSize; it++) {
        std::string setStr = "";
        int setNum = setNums[it];
        //setStr = std::string( std::max(0,setNameLen-1-int(log10(setNum)+0.00001)) , '0') + std::to_string(setNum);
        setStr = makeStrFromInt(setNum, setNameLen, '0');
        cout << "Starting analysis of set " << setStr << endl;
        data["sets"][setStr]["setNum"] = setNum;
        
        
        //SET UP ORIENTATION
        int orientationGame = getOrientationForExerciseGame( exerciseGame );
        setExerciseGameAndOrientation(exerciseGame, orientationGame);
        
        
        //INITIALIZE C++ WITH THE NUMBER OF REPS AND THE HAND
        //initialize((int) nrtbc_each, (int) hand);
        initialize((int) nrtbc_each, (int) hand, "{ \"segmentationAlg\": ["+std::to_string(segmentationAlg)+"]}");
        
        
        //SET UP IMAGES (ON ANDROID, WOULD SET UP AUDIO, BUT NO NEED TO SET UP AUDIO ON iOS)
        std::string pathToImages = "../../MoTrackAndroid/app/src/main/res/drawable/";
        setupImages(exercise_game, pathToImages);
        
        
        vector<float> IoUs;
        vector<float> accuracies;
        vector<float> paramDiffs;
        
        int frameCt = 0;
        int redboxCt = 0;
        int gameplayCt = 0;
        int warningCt = 0;
        std::string imgType = "";
        std::string newImgType = "A";
        std::string photoName = "-";
        Mat frameRaw, frameCV, frameIdealMask;
        while(frameCt >= 0) {
            frameCt++;
            if (imgType != newImgType) {
                photoName = "IMG_"+setStr+"_"+newImgType;
                frameRaw        = cv::imread(photoPath+photoName+"_RAW.JPG");
                frameCV         = cv::imread(photoPath+"IMG_"+setStr+"_"+newImgType+"_CV.JPG");
                frameIdealMask  = cv::imread(photoPath+"IMG_"+setStr+"_"+newImgType+"_RAW.png");
                
                
                if (frameRaw.data != NULL && frameIdealMask.data != NULL) {
                    data["sets"][setStr][photoName]["inputFrame"]["Wd"] = frameRaw.cols;
                    data["sets"][setStr][photoName]["inputFrame"]["Ht"] = frameRaw.rows;
                    data["sets"][setStr][photoName]["inputFrame"]["channelCt"] = frameRaw.channels();
                    
                    data["sets"][setStr][photoName]["inputMask"]["width"] = frameIdealMask.cols;
                    data["sets"][setStr][photoName]["inputMask"]["height"] = frameIdealMask.rows;
                    data["sets"][setStr][photoName]["inputMask"]["channelCt"] = frameIdealMask.channels();
                    
                    data["sets"][setStr][photoName]["letter"] = newImgType;
                    
                    data["sets"][setStr][photoName]["analysis"]["error"] = ""; //no error, but can be changed later
                    data["sets"][setStr][photoName]["analysis"]["warnings"] = {};
                    
                    
                    //double initScale = 300.0/frame.cols;
                    //cv::resize(frame, frame, cv::Size(), initScale, initScale);
                    
                    cv::cvtColor(frameRaw, frameRaw, COLOR_BGR2RGBA);
                    //cv::cvtColor(frame, frame, CV_RGB2RGBA); //make it four channels
                    
                    if (frameCV.data != NULL) {
                        cv::cvtColor(frameCV, frameCV, COLOR_BGR2RGBA);
                    } else {
                        frameCV = cv::Mat(cv::Size(frameRaw.cols,frameRaw.rows*1.25), CV_8UC4, CLR_BLACK_4UC);
                    }
                    
                } else {
                    cout << "Error, images are null for letter " << newImgType << " and set " << setStr << endl;
                    frameCt = -1;
                    continue;
                }
                
                imgType = newImgType;
            }
            
            
            if (show) cv::imshow("input", frameRaw);
            
            //can put -1 for frameCV.cols and frameCV.rows arguments instead if frameCV doesn't exist
            Mat frameOutput, frameProjMask;
            int outputWd = -1, outputHt = -1;
            if (frameCV.data != NULL) { //this should never be NULL anyway (because if it were null, the program would have exited/continued to the next set anyway), but here in case the situation changes in the future
                outputWd = frameCV.cols;
                outputHt = frameCV.rows;
            }
            std::tie(frameOutput, frameProjMask) = inputImage(frameRaw.clone(), outputWd, outputHt,  false);
            cv::resize(frameProjMask, frameProjMask, frameRaw.size()); //necessary because the mask has often been reduced in inputImage(.) for faster analysis
            
            if (show) {
                cv::imshow("model output", frameOutput);
                cv::imshow("actual output", frameCV);
            }
            
            cout << "|" << makeStrFromInt(frameCt, 4, ' ') << ") imgType=" << imgType << ", ";
            
            Mat frameIdealMaskGrey;
            int space = 1;
            // (67, 128, 11) or hex=43800b is the green that the masks are on
            cv::inRange(frameIdealMask, cv::Scalar(67-space,128-space,11-space,0), cv::Scalar(67+space,128+space,11+space,255), frameIdealMaskGrey);
            
            if (show) cv::imshow("mask", frameIdealMaskGrey);
            
            int area = frameIdealMask.rows * frameIdealMask.cols;
            
            Mat notframeProjMask, notframeIdealMaskGrey;
            cv::bitwise_not(frameProjMask, notframeProjMask);
            cv::bitwise_not(frameIdealMaskGrey, notframeIdealMaskGrey);
            
            /*
             int frameProjMaskCt = cv::countNonZero(frameProjMask);
             int frameIdealMaskGreyCt = cv::countNonZero(frameIdealMaskGrey);
             int notframeProjMaskCt = cv::countNonZero(notframeProjMask);
             int notframeIdealMaskGreyCt = cv::countNonZero(notframeIdealMaskGrey);*/
            
            
            Mat correctAcceptance, type1Error, type2Error, correctRejection, bothMasks;
            bitwise_and(   frameProjMask,    frameIdealMaskGrey, correctAcceptance);
            bitwise_and(notframeProjMask, notframeIdealMaskGrey, correctRejection);
            bitwise_and(   frameProjMask, notframeIdealMaskGrey, type1Error);
            bitwise_and(notframeProjMask,    frameIdealMaskGrey, type2Error);
            bitwise_or (   frameProjMask,    frameIdealMaskGrey, bothMasks);
            
            int correctAcceptanceCt = cv::countNonZero(correctAcceptance);
            int  correctRejectionCt = cv::countNonZero(correctRejection);
            int        type1ErrorCt = cv::countNonZero(type1Error);
            int        type2ErrorCt = cv::countNonZero(type2Error);
            int         bothMasksCt = cv::countNonZero(bothMasks);
            
            //Source for accuracy metrics: https://www.jeremyjordan.me/evaluating-image-segmentation-models/
            double correctAcceptanceFrac = correctAcceptanceCt*1.0/area;
            double correctRejectionFrac  = correctRejectionCt*1.0/area;
            double type1ErrorFrac        = type1ErrorCt*1.0/area;
            double type2ErrorFrac        = type2ErrorCt*1.0/area;
            double totalFrac = correctAcceptanceFrac + correctRejectionFrac + type1ErrorFrac + type2ErrorFrac; //should be 1
            
            
            double IoU = (bothMasksCt == 0 ? 1 : correctAcceptanceCt*1.0/bothMasksCt);
            //double accuracy = (totalFrac == 0 ? 0 : (correctAcceptanceFrac+correctRejectionFrac)/totalFrac); //totalFrac should always be 1
            double accuracy = correctAcceptanceFrac+correctRejectionFrac;
            cout << "IoU=" << IoU << ", ";
            cout << "Accr=" << accuracy << ", ";
            
            
            //cv::Mat frameProjMask = (frameRaw == frameOutput(cv::Rect(0,frameOutput.rows-frameRaw.rows,frameRaw.cols,frameRaw.rows)));
            //cv::imshow("proj mask", frameProjMask);
            
            cout << endl;
            
            
            
            if (currentStage == STAGE_BG_SUBTRACT_BEGINNING || currentStage == STAGE_BG_SUBTRACT_STANDARD) {
                if (frameCt > 10) doSkip();
                
            } else if (currentStage == STAGE_REDBOX1) {
                redboxCt++;
                
                if (redboxCt > 3 && imgType != "B") {
                    newImgType = "B";
                }
                if (redboxCt > 50) {
                    if (shouldSkipIfRedboxUnsolvable) {
                        warningCt++;
                        data["sets"][setStr][photoName]["analysis"]["warnings"].push_back( "The app has been in redbox for too long, frameCt=" + std::to_string(frameCt) + ", redboxCt=" + std::to_string(redboxCt) + ". Skipping ahead." );
                        doSkip();
                        
                    } else {
                        cout << "Error, the app has been in redbox too long for redboxCt=" << redboxCt << endl;
                        data["sets"][setStr][photoName]["analysis"]["error"] = "The app has been in redbox for too long, frameCt=" + std::to_string(frameCt) + ", redboxCt=" + std::to_string(redboxCt) + ".";
                        
                        Mat analysisResults;
                        
                        //Ooops. For some reason, the proj outputImage and the actual output from the phone (iPhone X) was stretched. The camera part (not counting the top text area) was 375 (cols) by 575 (rows).
                        //Will stretch our analysis so it is easier to make it align and compare
                        cv::resize(frameRaw, analysisResults, Size(375,575) );
                        
                        cv::copyMakeBorder(analysisResults, analysisResults, frameOutput.rows-analysisResults.rows, 0, 0, 0, BORDER_CONSTANT, CLR_WHITE_4UC);
                        
                        if (saveExtraInfo) {
                            hconcat(analysisResults, frameOutput, analysisResults);
                            hconcat(analysisResults, frameCV, analysisResults);
                        }
                        
                        cv::putText(analysisResults, "Alg. #" + std::to_string(segmentationAlg),
                                    cv::Point(0, analysisResults.rows*(0.08-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.002, CLR_BLACK_4UC);
                        cv::putText(analysisResults, ("frameCt="+std::to_string(frameCt)),
                                    cv::Point(0, analysisResults.rows*(0.17-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.002, CLR_BLACK_4UC);
                        cv::putText(analysisResults, ("redboxCt="+std::to_string(redboxCt)),
                                    cv::Point(0, analysisResults.rows*(0.25-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.002, CLR_BLACK_4UC);
                        
                        cv::cvtColor(analysisResults, analysisResults, COLOR_RGBA2BGRA);
                        if (show) cv::imshow("analysisResults", analysisResults);
                        std::string fileSaveName = "photos/photo_analysis/" + std::to_string(segmentationAlg) + saveSuffix + "/IMG_"+setStr+"_"+newImgType+"_COMP"+std::to_string(segmentationAlg)+saveSuffix+".png";
                        cv::imwrite( fileSaveName, analysisResults);
                        
                        frameCt = -1;
                        continue;
                    }
                    
                }
                
            } else if (currentStage == STAGE_GAMEPLAY) {
                gameplayCt++;
                
                int cuttoffStepA = 10, cuttoffStepB = 20;
                if (gameplayCt < cuttoffStepA && imgType != "A") {
                    newImgType = "A";
                }
                if (gameplayCt > cuttoffStepA && gameplayCt < cuttoffStepB && imgType != "B") {
                    newImgType = "B";
                }
                if (gameplayCt == cuttoffStepB && imgType != "C") {
                    newImgType = "C";
                }
                bool increaseLetter = frameCt%3==2; //if the letter should be increased (not counting to A and to B)
                if (gameplayCt==cuttoffStepA || gameplayCt==cuttoffStepB || (gameplayCt>cuttoffStepB && increaseLetter) ) {
                    cv::Mat analysisResults ( frameProjMask.size(), CV_8UC4, Scalar(0,0,0,0) );
                    
                    analysisResults.setTo(cv::Scalar(67,128,11,255), frameIdealMaskGrey);
                    analysisResults.setTo(cv::Scalar(0,0,255,255), frameProjMask);
                    analysisResults.setTo(cv::Scalar(34,64,133,255), correctAcceptance);
                    
                    
                    Mat analysisResultsEdited = analysisResults.clone();
                    cv::circle(analysisResultsEdited, lastFistMaxLoc, lastFistRadius, cv::Scalar(67+127,128+127,11+127,255), analysisResults.cols*0.01); //labeled circle
                    double paramLabel, paramEst = paramHistory.back(); //the parameter from the last inputImage
                    cv::Point dmaxLoc;
                    double radius;
                    int highest_y;
                    Mat frameDisplay = cv::Mat(frameIdealMask.size(), CV_8UC4, Scalar(0,0,0,255));
                    std::tie(paramLabel, dmaxLoc, radius, highest_y) = analyze_fist(frameIdealMaskGrey, frameDisplay, 0, 1, false);
                    cv::circle(analysisResultsEdited, dmaxLoc, radius, cv::Scalar(0+127,0+127,255,255), analysisResults.cols*0.01); //estimated circle
                    
                    cv::addWeighted(analysisResults, 0.7, analysisResultsEdited, 0.3, 0, analysisResults);
                    double paramDiff = subtractParams(paramEst,paramLabel);
                    
                    //Ooops. For some reason, the proj outputImage and the actual output from the phone (iPhone X) was stretched. The camera part (not counting the top text area) was 375 (cols) by 575 (rows).
                    //Will stretch our analysis so it is easier to make it align and compare
                    cv::resize(analysisResults, analysisResults, Size(375,575) );
                    
                    cv::copyMakeBorder(analysisResults, analysisResults, frameOutput.rows-analysisResults.rows, 0, 0, 0, BORDER_CONSTANT, CLR_WHITE_4UC);
                    
                    if (saveExtraInfo) {
                        hconcat(analysisResults, frameOutput, analysisResults);
                        hconcat(analysisResults, frameCV, analysisResults);
                    }
                    
                    
                    cv::putText(analysisResults, "Alg. #" + std::to_string(segmentationAlg),
                                cv::Point(0, analysisResults.rows*(0.05-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.001, CLR_BLACK_4UC);
                    cv::putText(analysisResults, ("IoU  = "+std::to_string(IoU)),
                                cv::Point(0, analysisResults.rows*(0.11-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.001, CLR_BLACK_4UC);
                    cv::putText(analysisResults, ("Accr = "+std::to_string(accuracy)),
                                cv::Point(0, analysisResults.rows*(0.14-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.001, CLR_BLACK_4UC);
                    cv::putText(analysisResults, "Param Est = " + paramToStr(paramEst),
                                cv::Point(0, analysisResults.rows*(0.19-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.001, CLR_BLACK_4UC);
                    cv::putText(analysisResults, "Param Lbl = " + paramToStr(paramLabel),
                                cv::Point(0, analysisResults.rows*(0.22-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.001, CLR_BLACK_4UC);
                    cv::putText(analysisResults, "Param Dif = " + std::to_string(paramDiff),
                                cv::Point(0, analysisResults.rows*(0.25-0.02) ), FONT_HERSHEY_TRIPLEX, analysisResults.rows*0.001, CLR_BLACK_4UC);
                    
                    cv::cvtColor(analysisResults, analysisResults, COLOR_RGBA2BGRA);
                    if (show) cv::imshow("analysisResults", analysisResults);
                    std::string fileSaveName = "photos/photo_analysis/" + std::to_string(segmentationAlg) + saveSuffix + "/IMG_"+setStr+"_"+newImgType+"_COMP"+std::to_string(segmentationAlg)+saveSuffix+".png";
                    cv::imwrite( fileSaveName, analysisResults);
                    cout << "Saving " << fileSaveName << endl;
                    
                    int correctAcceptanceCt = cv::countNonZero(frameProjMask);
                    int  correctRejectionCt = cv::countNonZero(correctRejection);
                    int        type1ErrorCt = cv::countNonZero(type1Error);
                    int        type2ErrorCt = cv::countNonZero(type2Error);
                    int         bothMasksCt = cv::countNonZero(bothMasks);
                    
                    
                    data["sets"][setStr][photoName]["analysis"]["IoU"] = IoU;
                    data["sets"][setStr][photoName]["analysis"]["accuracy"] = accuracy;
                    data["sets"][setStr][photoName]["analysis"]["paramLabel"] = paramLabel;
                    data["sets"][setStr][photoName]["analysis"]["paramEst"] = paramEst;
                    data["sets"][setStr][photoName]["analysis"]["paramDiff"] = paramDiff;
                    data["sets"][setStr][photoName]["analysis"]["paramDiffAbs"] = abs(paramDiff);
                    
                    
                    data["sets"][setStr][photoName]["analysis"]["correctAcceptanceCt"] = correctAcceptanceCt;
                    data["sets"][setStr][photoName]["analysis"]["correctRejectionCt"] = correctRejectionCt;
                    data["sets"][setStr][photoName]["analysis"]["type1ErrorCt"] = type1ErrorCt;
                    data["sets"][setStr][photoName]["analysis"]["type2ErrorCt"] = type2ErrorCt;
                    data["sets"][setStr][photoName]["analysis"]["bothMasksCt"] = bothMasksCt;
                    
                    data["sets"][setStr][photoName]["analysis"]["correctAcceptanceFrac"] = correctAcceptanceFrac;
                    data["sets"][setStr][photoName]["analysis"]["correctRejectionFrac"] = correctRejectionFrac;
                    data["sets"][setStr][photoName]["analysis"]["type1ErrorFrac"] = type1ErrorFrac;
                    data["sets"][setStr][photoName]["analysis"]["type2ErrorFrac"] = type2ErrorFrac;
                    data["sets"][setStr][photoName]["analysis"]["totalFrac"] = totalFrac;
                    
                    IoUs.push_back(IoU);
                    accuracies.push_back(accuracy);
                    paramDiffs.push_back(paramDiff);
                    
                    if (gameplayCt>cuttoffStepB && increaseLetter) {
                        char letter = imgType.at(0);
                        newImgType = std::string(1, 65+((letter-65)%26)+1);
                    }
                }
            } else {
                cout << "No stage" << endl;
            }
            
            
            char key = (char) cv::waitKey(30); //cast can be done implicitly, but showed for clarification
            if (key == 27 || key == 'q') break;
            else if (key == 'r') resetCalibrationBackground();
            else if (key == 's') {
                doSkip();
                cout << "Skip" << endl;
            } else if (key == ' ') cv::waitKey(0); //pause
            else if (key == KEY_NONE) { //no key pressed
                
            }
            
        }
        
        allIoUs.push_back(IoUs);
        allAccuracies.push_back(accuracies);
        allParamDiffs.push_back(paramDiffs);
        
        
        cout << "Ending analysis of set " << setStr << endl;
        cout << endl;
        
    }
    
    data["consolidatedData"]["allIoUs"] = allIoUs;
    data["consolidatedData"]["allAccuracies"] = allAccuracies;
    data["consolidatedData"]["paramDiffs"] = allParamDiffs;
    
    
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration time_span = nowTime - measurementStartTime;
    double time_diff = ((double) time_span.count())* std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
    cout << "Total Time: " << time_diff << endl;
    
    if (shouldSave) {
        std::string jsonUnanalyzedFileName = "photos/photo_analysis/" + std::to_string(segmentationAlg) + saveSuffix + "/data_unanalyzed_"+std::to_string(segmentationAlg)+saveSuffix+".json";
        std::ofstream unanalyzedFile(jsonUnanalyzedFileName);
        unanalyzedFile << data;
        cout << "Saved unanalyzed json data to file '" + jsonUnanalyzedFileName + "'. " << endl;
    }
    cout << endl;
    
    return data;
}


void testAlgorithmMeasurement() {
    std::string photoPath = "photos/real_photos/";
    std::string setStr = "0001";
    
    int nrtbc_each = 5;
    int hand = 0;
    int exerciseGame = FIST_PUMP_BALLOON_GAME;
    
    
    //SET UP ORIENTATION
    int orientationGame = getOrientationForExerciseGame( exerciseGame );
    setExerciseGameAndOrientation(exerciseGame, orientationGame);
    
    
    //INITIALIZE C++ WITH THE NUMBER OF REPS AND THE HAND
    //initialize((int) nrtbc_each, (int) hand);
    initialize((int) nrtbc_each, (int) hand, "{ \"segmentationAlg\": [1684]}");
    
    
    //SET UP IMAGES (ON ANDROID, WOULD SET UP AUDIO, BUT NO NEED TO SET UP AUDIO ON iOS)
    std::string pathToImages = "../../MoTrackAndroid/app/src/main/res/drawable/";
    setupImages(exercise_game, pathToImages);
    
    std::string imgType = "";
    std::string newImgType = "A";
    //Mat matA = cv::imread(photoPath+"IMG_"+set+"_"+letter+"_RAW.JPG");
    
    int frameCt = 0;
    
    Mat frameRaw, frameCV, frameIdealMask;
    while(true) {
        frameCt++;
        if (imgType != newImgType) {
            frameRaw  = cv::imread(photoPath+"IMG_"+setStr+"_"+newImgType+"_RAW.JPG");
            frameCV   = cv::imread(photoPath+"IMG_"+setStr+"_"+newImgType+"_CV.JPG");
            frameIdealMask  = cv::imread(photoPath+"IMG_"+setStr+"_"+newImgType+"_RAW.png");
            
            if (frameRaw.data != NULL && frameCV.data != NULL) {
                //double initScale = 300.0/frame.cols;
                //cv::resize(frame, frame, cv::Size(), initScale, initScale);
                
                cv::cvtColor(frameRaw, frameRaw, COLOR_BGR2RGBA);
                //cv::cvtColor(frame, frame, CV_RGB2RGBA); //make it four channels
                
                cv::cvtColor(frameCV, frameCV, COLOR_BGR2RGB);
                
            } else {
                cout << "Error, images are null" << endl;
            }
            
            imgType = newImgType;
        }
        
        
        if (frameRaw.data != NULL && frameCV.data != NULL) {
            cv::imshow("input", frameRaw);
            
            //can put -1 for frameCV.cols and frameCV.rows arguments instead if frameCV doesn't exist
            Mat frameOutput, frameProjMask;
            std::tie(frameOutput, frameProjMask) = inputImage(frameRaw.clone(), frameCV.cols, frameCV.rows,  false);
            cv::resize(frameProjMask, frameProjMask, frameRaw.size()); //necessary because the mask has often been reduced in inputImage(.) for faster analysis
            
            cv::imshow("model output", frameOutput);
            cv::imshow("actual output", frameCV);
            
            if (frameIdealMask.data != NULL) {
                Mat frameIdealMaskGrey;
                //cv::cvtColor(frameIdealMask, frameIdealMaskGrey, CV_RGB2GRAY);
                
                //Get all non-white pixels (i.e. the green pixels)
                /*
                 if (frameIdealMask.channels() == 3) cv::inRange(frameIdealMask, cv::Scalar(0,0,1), cv::Scalar(255,250,255), frameIdealMaskGrey);
                 else cv::inRange(frameIdealMask, cv::Scalar(0,0,0,1), cv::Scalar(255,250,255,255), frameIdealMaskGrey); //four channel is the one actually used right now
                 
                 cv::cvtColor(frameIdealMask, frameIdealMaskGrey, CV_RGB2GRAY);
                 cv::threshold(frameIdealMaskGrey, frameIdealMaskGrey, 200, 255, THRESH_BINARY);*/
                
                int space = 1;
                // (67, 128, 11) or hex=43800b is the green that the masks are on
                cv::inRange(frameIdealMask, cv::Scalar(67-space,128-space,11-space,0), cv::Scalar(67+space,128+space,11+space,255), frameIdealMaskGrey);
                
                cv::imshow("mask", frameIdealMaskGrey);
                
                int area = frameIdealMask.rows * frameIdealMask.cols;
                
                Mat notframeProjMask, notframeIdealMaskGrey;
                cv::bitwise_not(frameProjMask, notframeProjMask);
                cv::bitwise_not(frameIdealMaskGrey, notframeIdealMaskGrey);
                
                Mat correctAcceptance, type1Error, type2Error, correctRejection, bothMasks;
                bitwise_and(   frameProjMask,    frameIdealMaskGrey, correctAcceptance);
                bitwise_and(notframeProjMask, notframeIdealMaskGrey, correctRejection);
                bitwise_and(   frameProjMask, notframeIdealMaskGrey, type1Error);
                bitwise_and(notframeProjMask,    frameIdealMaskGrey, type2Error);
                bitwise_or (   frameProjMask,    frameIdealMaskGrey, bothMasks);
                
                int correctAcceptanceCt = cv::countNonZero(frameProjMask);
                int  correctRejectionCt = cv::countNonZero(correctRejection);
                int        type1ErrorCt = cv::countNonZero(type1Error);
                int        type2ErrorCt = cv::countNonZero(type2Error);
                int         bothMasksCt = cv::countNonZero(bothMasks);
                
                //Source for accuracy metrics: https://www.jeremyjordan.me/evaluating-image-segmentation-models/
                double correctAcceptanceFrac = correctAcceptanceCt*1.0/area;
                double correctRejectionFrac  = correctRejectionCt*1.0/area;
                double type1ErrorFrac        = type1ErrorCt*1.0/area;
                double type2ErrorFrac        = type2ErrorCt*1.0/area;
                double totalFrac = correctAcceptanceFrac + correctRejectionFrac + type1ErrorFrac + type2ErrorFrac;
                
                double IoU = (bothMasksCt == 0 ? 1 : correctAcceptanceCt*1.0/bothMasksCt);
                double accuracy = (totalFrac == 0 ? 0 : (correctAcceptanceFrac+correctRejectionFrac)/totalFrac);
                cout << "IoU=" << IoU << " ";
                cout << "Accr=" << accuracy << " ";
                //cout << correctAcceptanceFrac << " " << correctRejectionFrac << " " << type2ErrorFrac << " " << type1ErrorFrac << " => " << totalFrac << " ";
                
            }
            
            //cv::Mat frameProjMask = (frameRaw == frameOutput(cv::Rect(0,frameOutput.rows-frameRaw.rows,frameRaw.cols,frameRaw.rows)));
            //cv::imshow("proj mask", frameProjMask);
            
            //cout << "  |" << std::to_string(i) << ") imgType: " << imgType << endl;
            cout << "  |" << makeStrFromInt(frameCt, 4, ' ') << ") imgType=" << imgType << endl;
            
        }
        
        
        char key = (char) cv::waitKey(30); //cast can be done implicitly, but showed for clarification
        if (key == 27 || key == 'q') break;
        else if (key == 'r') resetCalibrationBackground();
        else if (key == 's') {
            doSkip();
            cout << "Skip" << endl;
        } else if (key == ' ') cv::waitKey(0); //pause
        else if (isalpha(key)) { //use shift/caps lock to make sure they come in as capital
            newImgType = std::string(1,key);
            
        } else if (key == KEY_LEFT) {
            //capitals between 65 and 90 inclusive
            char letter = imgType.at(0);
            newImgType = std::string(1, 65+((letter-65)%26)-1);
            
        } else if (key == KEY_RIGHT || frameCt==60) {
            char letter = imgType.at(0);
            newImgType = std::string(1, 65+((letter-65)%26)+1);
            
        } else if (key == KEY_NONE) { //no key pressed
            
        }
    }
}


json analyzeMeasurementData(json data, vector<int> setNums, int setNameLen) {
    vector< vector<double> > allIoUs = data["consolidatedData"]["allIoUs"];
    vector< vector<double> > allAccuracies = data["consolidatedData"]["allAccuracies"];
    //paramDiffs should be vector< vector<double> >, but if it is read from a file, and there is a null value (found from saving a nan), then it will throw a json error. If you define the type as auto, it works here, but you later cannot get it to compile when adding values to a element of it paramDiffs[it1][it2]
    //vector< vector<double> > paramDiffs = data["consolidatedData"]["paramDiffs"];
    //auto paramDiffs = data["consolidatedData"]["paramDiffs"];
    unsigned long totElements = 0;
    double totIoUs = 0;
    double totAccuracies = 0;
    //double totParamDiffsAbs = 0;
    int totWarningsCt = 0;
    int totErrorsCt = 0;
    size_t size1 = allIoUs.size();
    for (size_t it1 = 0; it1 < size1; it1++ ) {
        size_t size2 = allIoUs[it1].size();
        double setTotIoUs = 0;
        double setTotAccuracies = 0;
        //double setTotParamDiffsAbs = 0;
        int setElements = 0;
        int setWarningsCt = 0;
        int setErrorsCt = 0;
        
        std::string setStr = makeStrFromInt(setNums[it1], setNameLen, '0');
        
        /*
         cout << "IoUs for set " << setStr << " = { ";
         for (size_t it2 = 0; it2 < size2; it2++ ) {
         setTotIoUs += allIoUs[it1][it2];
         cout << allIoUs[it1][it2] << ", ";
         }
         cout << "}" << endl;
         
         cout << "Accuracies for set " << setStr << " = {";
         for (size_t it2 = 0; it2 < size2; it2++ ) {
         setTotAccuracies += allAccuracies[it1][it2];
         cout << allAccuracies[it1][it2] << ", ";
         }
         cout << "}" << endl;
         
         cout << "paramDiffs for set " << setStr << " = { ";
         size_t it2 = 0;
         for (; it2 < size2; it2++ ) {
         setTotParamDiffsAbs += abs(paramDiffs[it1][it2]);
         cout << paramDiffs[it1][it2] << ", ";
         }
         cout << "}" << endl;*/
        
        
        for (size_t it2 = 0; it2 < size2; it2++ ) {
            setTotIoUs += allIoUs[it1][it2];
            setTotAccuracies += allAccuracies[it1][it2];
            //setTotParamDiffsAbs += std::abs(paramDiffs[it1][it2]);
            setElements++;
        }
        
        for (size_t it2 = 0; it2 < size2; it2++ ) {
            std::string newImgType = std::string(1,'A'+it2); //'A' is a char, starts from A and is incremented upwards
            std::string photoName = "IMG_"+setStr+"_" +newImgType;
            auto warnings = data["sets"][setStr][photoName]["analysis"]["warnings"];
            unsigned long warningCt = warnings.size();
            setWarningsCt += warningCt;
            std::string error  = data["sets"][setStr][photoName]["analysis"]["error"];
            if (error.length() > 0) { //if the string is not empty
                setErrorsCt++;
            }
        }
        
        
        totElements += setElements;
        totIoUs += setTotIoUs;
        totAccuracies += setTotAccuracies;
        //totParamDiffsAbs += setTotParamDiffsAbs;
        totWarningsCt += setWarningsCt;
        totErrorsCt += setErrorsCt;
        
        
        data["sets"][setStr]["analysis"]["IoU"] = (setTotIoUs*1.0/totElements);
        data["sets"][setStr]["analysis"]["accuracy"] = (setTotAccuracies*1.0/totElements);
        //data["sets"][setStr]["analysis"]["paramDiffsAbs"] = (totParamDiffsAbs*1.0/totElements);
        data["sets"][setStr]["analysis"]["N"] = totElements;
        
        data["sets"][setStr]["analysis"]["warningsCt"] = setWarningsCt;
        data["sets"][setStr]["analysis"]["errorsCt"]   = setErrorsCt;
        
    }
    
    
    double avgIoU = (totIoUs/totElements);
    double avgAccuracy = (totAccuracies/totElements);
    //double avgParamDiffAbs = (totParamDiffsAbs/totElements);
    cout << "Avg IoU = " << avgIoU << endl;
    cout << "Avg Accr = " << avgAccuracy << endl;
    //cout << "Avg Accr = " << avgParamDiffAbs << endl;
    
    data["analysis"]["cppVersion"] = CPPVERSION;
    data["analysis"]["IoU"] = avgIoU;
    data["analysis"]["accuracy"] = avgAccuracy;
    data["analysis"]["N"] = totElements;
    
    data["analysis"]["warningsCt"] = totWarningsCt;
    data["analysis"]["errorsCt"]   = totErrorsCt;
    
    return data;
}
