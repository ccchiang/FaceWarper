#include "StdAfx.h"
#include "FaceDresser.h"
#include "Warper.h"


FaceDresser::FaceDresser(void)
{
}


FaceDresser::~FaceDresser(void)
{
}

Mat FaceDresser::Smoother(Mat& face, Mat& mask, int sigma1, int sigma2, int dist, int wsize)
{
	Mat tmpface, outface;
	outface = face.clone();
	tmpface = face.clone();
	bilateralFilter(face, tmpface, dist, sigma1, sigma2);
	medianBlur(tmpface, tmpface, wsize);
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
	Mat outface;
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

Mat FaceDresser::Reddish(Mat& face, Mat& mask, int offset)
{
	Mat outface;
	outface = face.clone();
	for (int i=0;i<outface.rows;i++) {
		for (int j=0;j<outface.cols;j++) {
			if (mask.at<uchar>(i,j)!=0) {
				float w = (float)(mask.at<uchar>(i,j))/255;
				int color = offset + outface.at<Vec3b>(i,j)[2];
				outface.at<Vec3b>(i,j)[2] = (uchar)(w*color+(1-w)*outface.at<Vec3b>(i,j)[2]);
			}
		}
	}
	return outface;
}

Mat FaceDresser::Blend(Face& face, Face& model, Mat& mask, float alpha)
{
	Mat outface, tmpface;
	outface = face.base_img.clone();
	tmpface = face.base_img.clone();
	Warper warper;
	FC fc = ALL;
	vector<Triangle> ts1 = face.getFCTriangles(&fc, 1);
	vector<Triangle> ts2 = model.getFCTriangles(&fc, 1);
	int n = ts1.size();
	for (int i=0;i<n;i++) 
		warper.WarpTriangle(&model.base_img, ts2[i], &tmpface, ts1[i], false); //做兩三角型之間的形變

	for (int i=0;i<outface.rows;i++) {
		for (int j=0;j<outface.cols;j++) {
			if (mask.at<uchar>(i,j)!=0) {
				float w = (float)(mask.at<uchar>(i,j))/255;
				uchar cr, cg, cb;
				cb = (uchar)(face.base_img.at<Vec3b>(i,j)[0]*alpha+(1-alpha)*tmpface.at<Vec3b>(i,j)[0]);
				cg = (uchar)(face.base_img.at<Vec3b>(i,j)[1]*alpha+(1-alpha)*tmpface.at<Vec3b>(i,j)[1]);
				cr = (uchar)(face.base_img.at<Vec3b>(i,j)[2]*alpha+(1-alpha)*tmpface.at<Vec3b>(i,j)[2]);
				outface.at<Vec3b>(i,j)[0] = (uchar)(w*cb+(1-w)*face.base_img.at<Vec3b>(i,j)[0]);
				outface.at<Vec3b>(i,j)[1] = (uchar)(w*cg+(1-w)*face.base_img.at<Vec3b>(i,j)[1]);
				outface.at<Vec3b>(i,j)[2] = (uchar)(w*cr+(1-w)*face.base_img.at<Vec3b>(i,j)[2]);
			}
		}
	}
	return outface;
}