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
			if (mask.at<uchar>(i,j)==255) {
				outface.at<Vec3b>(i,j)[0] = tmpface.at<Vec3b>(i,j)[0];
				outface.at<Vec3b>(i,j)[1] = tmpface.at<Vec3b>(i,j)[1];
				outface.at<Vec3b>(i,j)[2] = tmpface.at<Vec3b>(i,j)[2];
			}
		}
	}
	return outface;
}