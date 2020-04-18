//
//  main.h
//  MoTrack Therapy Mac
//
//  Created by Rahul Yerrabelli on March 5, 2019.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

#ifndef main_h
#define main_h


int main(int argc, const char * argv[]);

cv::Mat getImage(std::string path);

void setupImages(int exercise_game, std::string pathToImages);

void playSoundIfAny(std::string soundName);


#endif /* main_h */
