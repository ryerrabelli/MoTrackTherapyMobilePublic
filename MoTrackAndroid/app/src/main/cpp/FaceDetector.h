#pragma once

#include <opencv2/opencv.hpp>

/*
 Author: Pierfrancesco Soffritti https://github.com/PierfrancescoSoffritti
*/

using namespace cv;
using namespace std;

class FaceDetector {
	public:
		FaceDetector(void);
        void loadClassifier();
		void removeFaces(Mat input, Mat output, Mat frameDisplay, double ratio);
};
