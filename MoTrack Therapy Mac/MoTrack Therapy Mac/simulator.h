//
//  simulator.h
//  MoTrack Therapy Mac
//
//  Created by Rahul Yerrabelli on 6/21/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef simulator_h
#define simulator_h


#define KEY_NONE -1
#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_LEFT 2
#define KEY_RIGHT 3
#define CPPVERSION 13


#include <stdio.h>



int playGame();

cv::Mat getImage(std::string path);

void setupImages(int exercise_game, std::string pathToImages);

void playSoundIfAny(std::string soundName);



#endif /* simulator_h */
