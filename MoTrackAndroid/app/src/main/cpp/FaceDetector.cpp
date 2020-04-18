#include "FaceDetector.h"
#include <opencv2/opencv.hpp>

/*
 Author: Pierfrancesco Soffritti https://github.com/PierfrancescoSoffritti
*/

using namespace cv;


Rect getFaceRect(Mat input);

//String faceClassifierFileName = "../res/haarcascade_frontalface_alt.xml";
//String faceClassifierFileName = "../../MoTrackAndroid/app/src/main/res/drawable/haarcascade_frontalface_alt.xml";
String faceClassifierFileName = "haarcascade_frontalface_default.xml";
CascadeClassifier faceCascadeClassifier1;

FaceDetector::FaceDetector(void) {
	//if (!faceCascadeClassifier1.load(faceClassifierFileName))
	//	throw runtime_error("can't load file " + faceClassifierFileName);
}

void FaceDetector::loadClassifier() {
    if (!faceCascadeClassifier1.load(faceClassifierFileName))
        throw runtime_error("can't load file " + faceClassifierFileName);
    cout << "faceCascadeClassifier1.empty(): " << faceCascadeClassifier1.empty() << endl;
}

void FaceDetector::removeFaces(Mat frameEdit, Mat mask, Mat frameDisplay, double ratio) {
	vector<Rect> faces;
	Mat frameGray;

    cvtColor(frameEdit, frameGray, COLOR_BGR2GRAY);
	equalizeHist(frameGray, frameGray);

    faceCascadeClassifier1.detectMultiScale(frameGray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

    cout << "face count" << faces.size() << endl;
	for (size_t i = 0; i < faces.size(); i++) {
        cv::rectangle(
			mask,
			Point(faces[i].x, faces[i].y),
			Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height),
			Scalar(0),
			-1
		);
        cv::rectangle(
            frameDisplay,
            Point(faces[i].x/ratio, faces[i].y)/ratio,
            Point(faces[i].x/ratio + faces[i].width/ratio, faces[i].y/ratio + faces[i].height/ratio),
            Scalar(0, 0, 0, 255),
            -1
            );
	}
}

Rect getFaceRect(Mat input) {
	vector<Rect> faceRectangles;
	Mat inputGray;

	cvtColor(input, inputGray, COLOR_BGR2GRAY);
	equalizeHist(inputGray, inputGray);

    faceCascadeClassifier1.detectMultiScale(inputGray, faceRectangles, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

	if (faceRectangles.size() > 0)
		return faceRectangles[0];
	else
		return Rect(0, 0, 1, 1);
}
