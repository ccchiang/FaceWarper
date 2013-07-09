#include "StdAfx.h"
#include "FDetector.h"
#include <vector>

string file[N_FEATURES] = {"G:\\Program Files\\OpenCV245\\data\\haarcascades\\haarcascade_frontalface_alt_tree.xml",
				  "G:\\Program Files\\OpenCV245\\data\\haarcascades\\haarcascade_mcs_eyepair_big.xml",
				  "G:\\Program Files\\OpenCV245\\data\\haarcascades\\haarcascade_mcs_nose.xml",
				  "G:\\Program Files\\OpenCV245\\data\\haarcascades\\haarcascade_mcs_mouth.xml"};  
//CascadeClassifier FDetector::cascade_f;// = (CvHaarClassifierCascade*)cvLoad(file1.c_str(), 0, 0, 0);  
//CascadeClassifier FDetector::cascade_le;// = (CvHaarClassifierCascade*)cvLoad(file2.c_str(), 0, 0, 0);  
//CascadeClassifier FDetector::cascade_re;// = (CvHaarClassifierCascade*)cvLoad(file3.c_str(), 0, 0, 0);  
//CascadeClassifier FDetector::cascade_n;// = (CvHaarClassifierCascade*)cvLoad(file4.c_str(), 0, 0, 0);  
//CascadeClassifier FDetector::cascade_m;// = (CvHaarClassifierCascade*)cvLoad(file5.c_str(), 0, 0, 0);  

FDetector::FDetector(void)
{
	storage = cvCreateMemStorage(0);
	for (int i=0;i<N_FEATURES;i++)
		cascade_classifiers[i].load(file[i]);
}


FDetector::~FDetector(void)
{
	cvReleaseMemStorage(&storage);
}

vector<Rect> FDetector::DetectFacialFeature(Mat &img, int type, Rect* roi) {  
	Mat roi_img;
	if (roi==NULL)
		roi_img = img.clone();
	else
		roi_img = img(*roi);
	vector<Rect> faces;
	cascade_classifiers[type].detectMultiScale(roi_img, faces, 1.1, 2, 
		0|CV_HAAR_SCALE_IMAGE, Size(30, 30)); 
	if (roi!=NULL) {
		for (int i=0;i<(int)faces.size();i++) {
			faces[i].x += roi->x;
			faces[i].y += roi->y;
		}
	}
	return faces;
}  

MatchResult FDetector::Match(Mat& simg, Mat& dimg, Rect* roi1, Rect* roi2, int ftype,
	vector<KeyPoint>& keypoints1, vector<KeyPoint>& keypoints2,
	Mat& descriptors1, Mat& descriptors2, int n_kpts)
{
	Mat img1, img2;
	if (roi1!=NULL)
		img1 = simg(*roi1);
	else
		img1 = simg;
	if (roi2!=NULL)
		img2 = dimg(*roi2);
	else
		img2 = dimg;
	Mat gray1, gray2;
	cvtColor(img1, gray1, CV_BGR2GRAY);
	cvtColor(img2, gray2, CV_BGR2GRAY);
	SiftFeatureDetector siftdetector(n_kpts*200);
	SiftFeatureDetector surfdetector(n_kpts*200);
	GoodFeaturesToTrackDetector gftdetector(n_kpts, 0.01, 1.0, 2, true, 0.01);
	FastFeatureDetector ffdetector(n_kpts);
	//SIFT siftextractor;
	//SURF surfextractor;
	switch (ftype) {
	case 0: //SIFT
		siftdetector.detect(img1, keypoints1);
		siftdetector.detect(img2, keypoints2);
		if (keypoints1.size()!=0)
			siftdetector.compute(img1, keypoints1, descriptors1);
		if (keypoints2.size()!=0)
			siftdetector.compute(img2, keypoints2, descriptors2);
		break;
	case 1:
		surfdetector.detect(gray1, keypoints1);
		surfdetector.detect(gray2, keypoints2);
		if (keypoints1.size()!=0)
			siftdetector.compute(gray1, keypoints1, descriptors1);
		if (keypoints2.size()!=0)
			siftdetector.compute(gray2, keypoints2, descriptors2);
		break;
	case 2:
		gftdetector.detect(gray1, keypoints1);
		gftdetector.detect(gray2, keypoints2);
		if (keypoints1.size()!=0)
			siftdetector.compute(gray1, keypoints1, descriptors1);
		if (keypoints2.size()!=0)
			siftdetector.compute(gray2, keypoints2, descriptors2);
		break;
	case 3:
		ffdetector.detect(img1, keypoints1);
		ffdetector.detect(img2, keypoints2);
		if (keypoints1.size()!=0)
			siftdetector.compute(img1, keypoints1, descriptors1);
		if (keypoints2.size()!=0)
			siftdetector.compute(img2, keypoints2, descriptors2);
		break;
	}
	for (int i=0;i<(int)keypoints1.size();i++) {
		keypoints1[i].pt.x += roi1->x;
		keypoints1[i].pt.y += roi1->y;
	}
	for (int i=0;i<(int)keypoints2.size();i++) {
		keypoints2[i].pt.x += roi2->x;
		keypoints2[i].pt.y += roi2->y;
	}
	BruteForceMatcher<L2<float> > matcher;
	vector<DMatch> matches;
	MatchResult mr;
	if (!keypoints1.empty()&&!keypoints2.empty()) {
		matcher.match(descriptors1, descriptors2, matches);
		vector<KeyPoint> kps1;
		vector<KeyPoint> kps2;
		for (int i=0;i<(int)matches.size();i++) {
			int qid = matches[i].queryIdx;
			int tid = matches[i].trainIdx;
			kps1.push_back(keypoints1[qid]);
			kps2.push_back(keypoints2[tid]);
		}
		for (int i=0;i<(int)matches.size();i++){
			matches[i].queryIdx = matches[i].trainIdx = i;
		}
		mr.matches = matches;
		mr.qkps = kps1;
		mr.tkps = kps2;
	}
	////copy(matches.begin(), matches.end(), mr.matches.begin());
	//copy(kps1.begin(), kps1.end(), mr.qkps.begin());
	//copy(kps2.begin(), kps2.end(), mr.tkps.begin());
	return mr;
}

MatchResult FDetector::RegisterFaces(Mat img, Mat rimg)
{
	FDetector fd;
	//Detecting faces on both images first
	vector<Rect> faces = fd.DetectFacialFeature(img, 0, NULL);
	vector<Rect> rfaces = fd.DetectFacialFeature(rimg, 0, NULL);

	//Detecting the eyes within a specified region of interest on each image
	Rect eyes_roi(faces[0].x, faces[0].y, faces[0].width, faces[0].height/2);
	Rect reyes_roi(rfaces[0].x, rfaces[0].y, rfaces[0].width, rfaces[0].height/2);
	vector<Rect> eyes = fd.DetectFacialFeature(img, 1, &eyes_roi);
	vector<Rect> reyes = fd.DetectFacialFeature(rimg, 1, &reyes_roi);

	//Detecting the mouth within a specified region of interest on each image
	Rect mouth_roi(faces[0].x, faces[0].y+2*faces[0].height/3, 
		faces[0].width, faces[0].height/3);
	Rect rmouth_roi(rfaces[0].x, rfaces[0].y+2*rfaces[0].height/3, 
		rfaces[0].width, rfaces[0].height/3);
	vector<Rect> mouth = fd.DetectFacialFeature(img, 3, &mouth_roi);
	vector<Rect> rmouth = fd.DetectFacialFeature(rimg, 3, &rmouth_roi);

	//Detecting nose within a specified region of interest on each image
	Rect nose_roi(eyes[0].x, eyes[0].y+eyes[0].height, 
		eyes[0].width, mouth[0].y-eyes[0].y);
	Rect rnose_roi(reyes[0].x, reyes[0].y+reyes[0].height, 
		reyes[0].width, rmouth[0].y-reyes[0].y);
	vector<Rect> nose = fd.DetectFacialFeature(img, 2, &nose_roi);
	vector<Rect> rnose = fd.DetectFacialFeature(rimg, 2, &rnose_roi);

	// Finding sailent feature points withing the region of each facial component
	vector<KeyPoint> kps1, kps2;
	vector<KeyPoint> allkps1, allkps2;
	Mat ds1, ds2;
	MatchResult mr;
	int feature_type = 1; // Use the GoodFeatureToTrack features of OpenCV
	// Detecting and matching features within a ROI (left boundary) of the left eye
	Rect r1 = Rect(eyes[0].x, eyes[0].y+eyes[0].height/4, eyes[0].width/5,2*eyes[0].height/3);
	Rect r2 = Rect(reyes[0].x, reyes[0].y+reyes[0].height/4, reyes[0].width/5,2*reyes[0].height/3);
	mr = fd.Match(img, rimg, &r1, &r2, feature_type, kps1, kps2, ds1, ds2, 1);
	// Detecting and matching features within a ROI (right boundary) of the right eye
	r1 = Rect(eyes[0].x+4*(eyes[0].width)/5, eyes[0].y+eyes[0].height/4, eyes[0].width/5,2*eyes[0].height/3);
	r2 = Rect(reyes[0].x+4*(reyes[0].width)/5, reyes[0].y+reyes[0].height/4, reyes[0].width/5,2*reyes[0].height/3);
	mr += fd.Match(img, rimg, &r1, &r2, feature_type, kps1, kps2, ds1, ds2, 1);
	// Detecting and matching features within a ROI (left boundary) of the mouth
	r1 = Rect(mouth[0].x, mouth[0].y, mouth[0].width/5,mouth[0].height);
	r2 = Rect(rmouth[0].x, rmouth[0].y, rmouth[0].width/5,rmouth[0].height);
	mr += fd.Match(img, rimg, &r1, &r2, feature_type, kps1, kps2, ds1, ds2, 1);
	// Detecting and matching features within a ROI (right boundary) of the mouth
	r1 = Rect(mouth[0].x+4*(mouth[0].width)/5, mouth[0].y, mouth[0].width/5,mouth[0].height);
	r2 = Rect(rmouth[0].x+4*(rmouth[0].width)/5, rmouth[0].y, rmouth[0].width/5,rmouth[0].height);
	mr += fd.Match(img, rimg, &r1, &r2, feature_type, kps1, kps2, ds1, ds2, 1);
	// Detecting and matching features within a ROI (lower-right boundary) of the nose
	r1 = Rect(nose[0].x, nose[0].y+nose[0].height/2, nose[0].width/2, nose[0].height/2);
	r2 = Rect(rnose[0].x, rnose[0].y+rnose[0].height/2, rnose[0].width/2, rnose[0].height/2);
	mr += fd.Match(img, rimg, &r1, &r2, feature_type, kps1, kps2, ds1, ds2, 1);
	// Detecting and matching features within a ROI (lower-left region) of the nose
	r1 = Rect(nose[0].x+nose[0].width/2, nose[0].y+nose[0].height/2, nose[0].width/2, nose[0].height/2);
	r2 = Rect(rnose[0].x+rnose[0].width/2, rnose[0].y+rnose[0].height/2, rnose[0].width/2, rnose[0].height/2);
	mr += fd.Match(img, rimg, &r1, &r2, feature_type, kps1, kps2, ds1, ds2, 1);

	// drawing the results
	namedWindow("matches", 1);
	Mat img_matches;
	drawMatches(img, mr.qkps, rimg, mr.tkps, mr.matches, img_matches);
	imshow("matches", img_matches);
	return mr;
}
