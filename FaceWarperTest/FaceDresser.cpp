#include "StdAfx.h"
#include "FaceDresser.h"


FaceDresser::FaceDresser(void)
{
}


FaceDresser::~FaceDresser(void)
{
}

Mat FaceDresser::Smoother(Mat& face, Mat& mask)
{
	Mat tmpface, outface;
	outface = face.clone();
	tmpface = face.clone();
	bilateralFilter(face, tmpface, 5, 40, 40);
	medianBlur(tmpface, tmpface, 3);
	for (int i=0;i<outface.rows;i++) {
		for (int j=0;j<outface.cols;j++) {
			if (mask.at<uchar>(i,j)!=0) {
				float w = (float)(mask.at<uchar>(i,j))/255;
				outface.at<Vec3b>(i,j)[0] = (uchar)(w*tmpface.at<Vec3b>(i,j)[0]+(1-w)*outface.at<Vec3b>(i,j)[0]);
				outface.at<Vec3b>(i,j)[1] = (uchar)(w*tmpface.at<Vec3b>(i,j)[1]+(1-w)*outface.at<Vec3b>(i,j)[1]);
				outface.at<Vec3b>(i,j)[2] = (uchar)(w*tmpface.at<Vec3b>(i,j)[2]+(1-w)*outface.at<Vec3b>(i,j)[2]);
			}
		}
	}
	return outface;
}

Mat FaceDresser::Whiten(Mat& face, Mat& mask, double gamma)
{
	Mat tmpface, outface;
	double factor = 255/pow(255, gamma);
	outface = face.clone();
	for (int i=0;i<outface.rows;i++) {
		for (int j=0;j<outface.cols;j++) {
			if (mask.at<uchar>(i,j)!=0) {
				float w = (float)(mask.at<uchar>(i,j))/255;
				outface.at<Vec3b>(i,j)[0] = (uchar)(w*pow((double)face.at<Vec3b>(i,j)[0],gamma)*factor+
													(1-w)*outface.at<Vec3b>(i,j)[0]);
				outface.at<Vec3b>(i,j)[1] = (uchar)(w*pow((double)face.at<Vec3b>(i,j)[1],gamma)*factor+
													(1-w)*outface.at<Vec3b>(i,j)[1]);
				outface.at<Vec3b>(i,j)[2] = (uchar)(w*pow((double)face.at<Vec3b>(i,j)[2],gamma)*factor+
													(1-w)*outface.at<Vec3b>(i,j)[2]);;
			}
		}
	}
	return outface;
}