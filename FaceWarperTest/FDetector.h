#pragma once
#include <iostream>
#include <fstream>
#include <opencv2\core\core.hpp>
#include <opencv2\legacy\legacy.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\nonfree\features2d.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\objdetect\objdetect.hpp>
#include <vector>
#define N_FEATURES 4
#define FACE_RGN 0
#define EYE_RGN 1
#define NOSE_RGN 2
#define MOUTH_RGN 3

using namespace std;
using namespace cv;

class MatchResult {
public:
	vector<DMatch> matches;
	vector<KeyPoint> qkps; //Query keypoints
	vector<KeyPoint> tkps; //Train keypoints
	MatchResult& operator += (MatchResult& mr) {
		for (int i=0;i<(int)mr.matches.size();i++) {
			mr.matches[i].queryIdx += qkps.size();
			mr.matches[i].trainIdx += tkps.size();
		}
		matches.insert(matches.end(), mr.matches.begin(), mr.matches.end());
		qkps.insert(qkps.end(), mr.qkps.begin(), mr.qkps.end());
		tkps.insert(tkps.end(), mr.tkps.begin(), mr.tkps.end());
		return *this;
	}
};

class FDetector
{
public:
	CascadeClassifier cascade_classifiers[5];  
	CvMemStorage   *storage;  

public:
	FDetector(void);
	~FDetector(void);
	vector<Rect> DetectFacialFeature(Mat& img, int type, Rect* roi);
	MatchResult Match(Mat& img1, Mat& img2, Rect* roi1, Rect* roi2, int ftype,
		vector<KeyPoint>& keypoints1, vector<KeyPoint>& keypoints2,
		Mat& descriptors1, Mat& descriptors2, int n_kpts);
	MatchResult RegisterFaces(Mat srcimg, Mat dstimg);
};

