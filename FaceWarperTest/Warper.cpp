#include <set>
#include "StdAfx.h"
#include "Warper.h"
#include <cmath>

extern int FC_Vertices[9][64];
//FC AlignGroups[][5]={
//	{LEYE, REYE, NOSE, MOUTH, (FC)-1},
//	{LEYE, REYE, (FC)-1},
////	{MOUTH, NOSE, (FC)-1},
//};
//FC AlignGroups[][5]={
//	{LEYE, (FC)-1},
//	{REYE, (FC)-1},
//	{NOSE, (FC)-1},
//	{MOUTH, (FC)-1}
//};
//FC AlignGroups[][5]={
//	{LEYE, REYE, NOSE, MOUTH, (FC)-1},
//};
FC AlignGroups[][5]={
	{LEYE, REYE, NOSE, (FC)-1},
	{REYE, (FC)-1},
	{MOUTH, NOSE, (FC)-1},
//	{MOUTH, (FC)-1}
};
FC FCS[] = {LEYE, REYE, NOSE, MOUTH, JAW};


Warper::Warper(void)
{
	alpha = 0.015;
}

Warper::~Warper(void)
{
}

Mat Warper::FindAffine(Triangle & t1, Triangle & t2)
{
	Mat warp_mat = getAffineTransform(&t1.getVertex(0), &t2.getVertex(0));
	return warp_mat;
}

void Warper::FindLRBndry(float y, Triangle &dst, float* left, float* right)
{
	LineEquation l(0.0f, 1.0f, y);
	Point2f p;
	bool f;
	*left = INF;
	*right = -INF;
	f = dst.Intersect(l, 0, 1, &p);
	if (f) {
		*left = min(*left, p.x);
		*right = max(*right, p.x);
	}
	f = dst.Intersect(l, 0, 2, &p);
	if (f) {
		*left = min(*left, p.x);
		*right = max(*right, p.x);
	}
	f = dst.Intersect(l, 1, 2, &p);
	if (f) {
		*left = min(*left, p.x);
		*right = max(*right, p.x);
	}
}

Mat Warper::AffineAlign(vector<Point2f> *src_v, vector<Point2f> *dst_v)
{
	int n_v = src_v->size();
	Mat A(2*n_v, 6, CV_32F);
	Mat b(2*n_v, 1, CV_32F);
	Mat x(6, 1, CV_32F);
	Mat S(2, 3, CV_32F);
	for (int i=0;i<2*n_v;i++) {
		if (i%2==0) {
			A.at<float>(i,0) = (*src_v)[i/2].x;
			A.at<float>(i,1) = (*src_v)[i/2].y;
			A.at<float>(i,2) = 1;
			A.at<float>(i,3) = 0;
			A.at<float>(i,4) = 0;
			A.at<float>(i,5) = 0;
			b.at<float>(i,0) = (*dst_v)[i/2].x;
		}
		else {
			A.at<float>(i,0) = 0;
			A.at<float>(i,1) = 0;
			A.at<float>(i,2) = 0;
			A.at<float>(i,3) = (*src_v)[i/2].x;
			A.at<float>(i,4) = (*src_v)[i/2].y;
			A.at<float>(i,5) = 1;
			b.at<float>(i,0) = (*dst_v)[i/2].y;
		}
	}
	solve(A, b, x, DECOMP_NORMAL);
	S.at<float>(0, 0) = x.at<float>(0, 0);
	S.at<float>(0, 1) = x.at<float>(1, 0);
	S.at<float>(0, 2) = x.at<float>(2, 0);
	S.at<float>(1, 0) = x.at<float>(3, 0);
	S.at<float>(1, 1) = x.at<float>(4, 0);
	S.at<float>(1, 2) = x.at<float>(5, 0);
	return S;
}

Mat Warper::SimilarityAlign(vector<Point2f> *src_v, vector<Point2f> *dst_v)
{
	int n_v = src_v->size();
	Mat A(2*n_v, 4, CV_32F);
	Mat b(2*n_v, 1, CV_32F);
	Mat x(4, 1, CV_32F);
	Mat S(2, 3, CV_32F);
	for (int i=0;i<2*n_v;i++) {
		if (i%2==0) {
			A.at<float>(i,0) = (*src_v)[i/2].x;
			A.at<float>(i,1) = (*src_v)[i/2].y;
			A.at<float>(i,2) = 1;
			A.at<float>(i,3) = 0;
			b.at<float>(i,0) = (*dst_v)[i/2].x;
		}
		else {
			A.at<float>(i,0) = (*src_v)[i/2].y;
			A.at<float>(i,1) = -(*src_v)[i/2].x;
			A.at<float>(i,2) = 0;
			A.at<float>(i,3) = 1;
			b.at<float>(i,0) = (*dst_v)[i/2].y;
		}
	}
	solve(A, b, x, DECOMP_SVD);
	S.at<float>(0, 0) = x.at<float>(0, 0);
	S.at<float>(0, 1) = x.at<float>(1, 0);
	S.at<float>(0, 2) = x.at<float>(2, 0);
	S.at<float>(1, 0) = -x.at<float>(1, 0);
	S.at<float>(1, 1) = x.at<float>(0, 0);
	S.at<float>(1, 2) = x.at<float>(3, 0);
	return S;
}

/*
*/
vector<Mat> Warper::AlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2)
{
	int n = ts1.size();
	vector<Mat> transforms;
	Mat C1, C2;
	Mat m1, m2;
	Mat evec1, evec2;
	Mat eval1, eval2;
	vector<Mat> pixels1, pixels2;
	vector<Point2d> coordinates;
	Mat tmp_img(img1);
	for (int i=0;i<n;i++) {
		int min_y1 = (int)ts1[i].getMinY();
		int max_y1 = (int)ts1[i].getMaxY();
		int min_y2 = (int)ts2[i].getMinY();
		int max_y2 = (int)ts2[i].getMaxY();
		float left, right;
		for (int y=min_y1; y<=max_y1; y++) {
			FindLRBndry((float)y, ts1[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img1.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = img1.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = img1.at<Vec3b>(y, x)[2];
				pixels1.push_back(p);
				coordinates.push_back(Point2d((int)x, y));
			}
		}
		for (int y=min_y2; y<=max_y2; y++) {
			FindLRBndry((float)y, ts2[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img2.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = img2.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = img2.at<Vec3b>(y, x)[2];
				pixels2.push_back(p);
			}
		}
	}
	//ofstream f1("data1.txt"), f2("data2.txt");
	//for (int j=0;j<(int)pixels1.size();j++) {
	//	f1 << pixels1[j].at<double>(0,0) << "\t" 
	//		<< pixels1[j].at<double>(0,1) << "\t"
	//		<< pixels1[j].at<double>(0,2) << endl;
	//}
	//for (int j=0;j<(int)pixels2.size();j++) {
	//	f2 << pixels2[j].at<double>(0,0) << "\t" 
	//		<< pixels2[j].at<double>(0,1) << "\t"
	//		<< pixels2[j].at<double>(0,2) << endl;
	//}
	//f1.close();
	//f2.close();
	calcCovarMatrix(&pixels1[0], pixels1.size(), C1, m1, CV_COVAR_NORMAL);
	C1 = C1/(pixels1.size()-1);
	eigen(C1, eval1, evec1);
	cout << "C1=" << C1 << endl;
	cout << "vec1 = " << evec1 << endl;
	cout << "val1 = " << eval1 << endl;
	cout << "m1 = " << m1 << endl;
	calcCovarMatrix(&pixels2[0], pixels2.size(), C2, m2, CV_COVAR_NORMAL); 
	C2 = C2/(pixels2.size()-1);
	eigen(C2, eval2, evec2);
	Mat vec1, vec2;
	vec1 = evec1.row(0).cross(evec1.row(1));
	vec2 = evec2.row(0).cross(evec2.row(1));
	if (vec1.dot(evec1.row(2))>0)
		evec1.row(2) = -evec1.row(2);
	if (vec2.dot(evec2.row(2))>0)
		evec2.row(2) = -evec2.row(2);
	cout << "C2=" << C2 << endl;
	cout << "vec2 = " << evec2 << endl;
	cout << "val2 = " << eval2 << endl;
	cout << "m2 = " << m2 << endl;
	Mat Tt1(4, 4, CV_32F, Scalar(0));
	Mat Tr1(4, 4, CV_32F, Scalar(0));
	Mat Ts1(4, 4, CV_32F, Scalar(0));
	Mat Tt2(4, 4, CV_32F, Scalar(0));
	Mat Tr2(4, 4, CV_32F, Scalar(0));
	Tt1.at<float>(0,0) = 1.0f;
	Tt1.at<float>(1,1) = 1.0f;
	Tt1.at<float>(2,2) = 1.0f;
	Tt1.at<float>(0,3) = -(float)m1.at<double>(0,0);
	Tt1.at<float>(1,3) = -(float)m1.at<double>(0,1);
	Tt1.at<float>(2,3) = -(float)m1.at<double>(0,2);
	Tt1.at<float>(3,3) = 1.0f;
	cout << "Tt1 = " << Tt1 << endl;
	Tt2.at<float>(0,0) = 1.0f;
	Tt2.at<float>(1,1) = 1.0f;
	Tt2.at<float>(2,2) = 1.0f;
	Tt2.at<float>(0,3) = (float)m2.at<double>(0,0);
	Tt2.at<float>(1,3) = (float)m2.at<double>(0,1);
	Tt2.at<float>(2,3) = (float)m2.at<double>(0,2);
	Tt2.at<float>(3,3) = 1.0f;
	cout << "Tt2 = " << Tt2 << endl;
	Tr1.at<float>(0,0) = (float)evec1.at<double>(0, 0);
	Tr1.at<float>(0,1) = (float)evec1.at<double>(0, 1);
	Tr1.at<float>(0,2) = (float)evec1.at<double>(0, 2);
	Tr1.at<float>(1,0) = (float)evec1.at<double>(1, 0);
	Tr1.at<float>(1,1) = (float)evec1.at<double>(1, 1);
	Tr1.at<float>(1,2) = (float)evec1.at<double>(1, 2);
	Tr1.at<float>(2,0) = (float)evec1.at<double>(2, 0);
	Tr1.at<float>(2,1) = (float)evec1.at<double>(2, 1);
	Tr1.at<float>(2,2) = (float)evec1.at<double>(2, 2);
	Tr1.at<float>(3,3) = 1.0f;
	cout << "Tr1 = " << Tr1 << endl;
	Tr2.at<float>(0,0) = (float)evec2.at<double>(0, 0);
	Tr2.at<float>(1,0) = (float)evec2.at<double>(0, 1);
	Tr2.at<float>(2,0) = (float)evec2.at<double>(0, 2);
	Tr2.at<float>(0,1) = (float)evec2.at<double>(1, 0);
	Tr2.at<float>(1,1) = (float)evec2.at<double>(1, 1);
	Tr2.at<float>(2,1) = (float)evec2.at<double>(1, 2);
	Tr2.at<float>(0,2) = (float)evec2.at<double>(2, 0);
	Tr2.at<float>(1,2) = (float)evec2.at<double>(2, 1);
	Tr2.at<float>(2,2) = (float)evec2.at<double>(2, 2);
	Tr2.at<float>(3,3) = 1.0f;
	cout << "Tr2 = " << Tr2 << endl;
	Ts1.at<float>(0,0) = (float)sqrt(eval2.at<double>(0, 0)/eval1.at<double>(0, 0));
	Ts1.at<float>(1,1) = (float)sqrt(eval2.at<double>(1, 0)/eval1.at<double>(1, 0));
	Ts1.at<float>(2,2) = (float)sqrt(eval2.at<double>(2, 0)/eval1.at<double>(2, 0));
	Ts1.at<float>(3,3) = 1.0f;
	cout << "Ts1 = " << Ts1 << endl;
	Mat T = Tt2*Tr2.t()*Ts1*Tr1*Tt1;
	cout << T << endl;
	cout << T.inv() << endl;
	transforms.push_back(T);
	//for (int j=0; j<(int)pixels1.size(); j++) {
	//	int x = (int)coordinates[j].x;
	//	int y = (int)coordinates[j].y;
	//	Mat color(4, 1, CV_32F);
	//	color.at<float>(0,0) = (float)pixels1[j].at<double>(0,0);//(float)tmp_img.at<Vec3b>(y,x)[0];////img1.at<Vec3b>(y,x)[0];
	//	color.at<float>(1,0) = (float)pixels1[j].at<double>(0,1);//(float)tmp_img.at<Vec3b>(y,x)[1];//
	//	color.at<float>(2,0) = (float)pixels1[j].at<double>(0,2);//(float)tmp_img.at<Vec3b>(y,x)[2];//
	//	color.at<float>(3,0) = 1.0f;
	//	Mat new_color = T*color;	
	//	img1.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(new_color.at<float>(0,0));
	//	img1.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(new_color.at<float>(1,0));
	//	img1.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(new_color.at<float>(2,0));
	//}
	return transforms;
}


vector<Mat> Warper::NewAlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2)
{
	int n = ts1.size();
	vector<Mat> transforms;
	Mat C1, C2;
	Mat m1, m2;
	Mat evec1, evec2;
	Mat eval1, eval2;
	vector<Mat> pixels1, pixels2;
	vector<Point2d> coordinates;
	Mat tmp_img(img1);
	for (int i=0;i<n;i++) {
		int min_y1 = (int)ts1[i].getMinY();
		int max_y1 = (int)ts1[i].getMaxY();
		int min_y2 = (int)ts2[i].getMinY();
		int max_y2 = (int)ts2[i].getMaxY();
		float left, right;
		for (int y=min_y1; y<=max_y1; y++) {
			FindLRBndry((float)y, ts1[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img1.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = img1.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = img1.at<Vec3b>(y, x)[2];
				pixels1.push_back(p);
				coordinates.push_back(Point2d((int)x, y));
			}
		}
		for (int y=min_y2; y<=max_y2; y++) {
			FindLRBndry((float)y, ts2[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img2.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = img2.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = img2.at<Vec3b>(y, x)[2];
				pixels2.push_back(p);
			}
		}
	}
	calcCovarMatrix(&pixels1[0], pixels1.size(), C1, m1, CV_COVAR_NORMAL);
	C1 = C1/(pixels1.size()-1);
	calcCovarMatrix(&pixels2[0], pixels2.size(), C2, m2, CV_COVAR_NORMAL);
	C2 = C2/(pixels2.size()-1);

	Mat Tt1(4, 4, CV_32F, Scalar(0));
	Mat Ts1(4, 4, CV_32F, Scalar(0));
	Mat Tt2(4, 4, CV_32F, Scalar(0));
	Tt1.at<float>(0,0) = 1.0f;
	Tt1.at<float>(1,1) = 1.0f;
	Tt1.at<float>(2,2) = 1.0f;
	Tt1.at<float>(0,3) = -(float)m1.at<double>(0,0);
	Tt1.at<float>(1,3) = -(float)m1.at<double>(0,1);
	Tt1.at<float>(2,3) = -(float)m1.at<double>(0,2);
	Tt1.at<float>(3,3) = 1.0f;
	Tt2.at<float>(0,0) = 1.0f;
	Tt2.at<float>(1,1) = 1.0f;
	Tt2.at<float>(2,2) = 1.0f;
	Tt2.at<float>(0,3) = (float)m2.at<double>(0,0);
	Tt2.at<float>(1,3) = (float)m2.at<double>(0,1);
	Tt2.at<float>(2,3) = (float)m2.at<double>(0,2);
	Tt2.at<float>(3,3) = 1.0f;
	Ts1.at<float>(0,0) = (float)sqrt(C2.at<double>(0, 0)/C1.at<double>(0, 0));
	Ts1.at<float>(1,1) = (float)sqrt(C2.at<double>(1, 0)/C1.at<double>(1, 0));
	Ts1.at<float>(2,2) = (float)sqrt(C2.at<double>(2, 0)/C1.at<double>(2, 0));
	Ts1.at<float>(3,3) = 1.0f;
	Mat T = Tt2*Ts1*Tt1;
	cout << Tt2 << endl;
	cout << Ts1 << endl;
	cout << Tt1 << endl;
	cout << T << endl;
	transforms.push_back(T);
	//for (int j=0; j<(int)pixels1.size(); j++) {
	//	int x = (int)coordinates[j].x;
	//	int y = (int)coordinates[j].y;
	//	Mat color(4, 1, CV_32F);
	//	color.at<float>(0,0) = (float)pixels1[j].at<double>(0,0);//(float)tmp_img.at<Vec3b>(y,x)[0];////img1.at<Vec3b>(y,x)[0];
	//	color.at<float>(1,0) = (float)pixels1[j].at<double>(0,1);//(float)tmp_img.at<Vec3b>(y,x)[1];//
	//	color.at<float>(2,0) = (float)pixels1[j].at<double>(0,2);//(float)tmp_img.at<Vec3b>(y,x)[2];//
	//	color.at<float>(3,0) = 1.0f;
	//	Mat new_color = T*color;	
	//	img1.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(new_color.at<float>(0,0));
	//	img1.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(new_color.at<float>(1,0));
	//	img1.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(new_color.at<float>(2,0));
	//}
	return transforms;
}

vector<Mat> Warper::AlignColorLMS(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2)
{
	Mat tmp_img(img1);
	int n = ts1.size();
	vector<Mat> transforms;
	Mat C1, C2;
	Mat m1(1, 3, CV_64F), m2(1, 3, CV_64F);
	vector<Mat> pixels1, pixels2;
	Mat A(3*n, 12, CV_64F, Scalar(0));
	Mat b(3*n, 1, CV_64F);
	Mat x(12, 1, CV_64F);
	for (int i=0;i<n;i++) {
		int xx1 = (int)(ts1[i].getVertex(0).x + ts1[i].getVertex(1).x + ts1[i].getVertex(2).x)/3;
		int yy1 = (int)(ts1[i].getVertex(0).y + ts1[i].getVertex(1).y + ts1[i].getVertex(2).y)/3;
		int xx2 = (int)(ts2[i].getVertex(0).x + ts2[i].getVertex(1).x + ts2[i].getVertex(2).x)/3;
		int yy2 = (int)(ts2[i].getVertex(0).y + ts2[i].getVertex(1).y + ts2[i].getVertex(2).y)/3;
		m1.at<double>(0,0) = img1.at<Vec3b>(yy1,xx1)[0];
		m1.at<double>(0,1) = img1.at<Vec3b>(yy1,xx1)[1];
		m1.at<double>(0,2) = img1.at<Vec3b>(yy1,xx1)[2];
		m2.at<double>(0,0) = img2.at<Vec3b>(yy2,xx2)[0];
		m2.at<double>(0,1) = img2.at<Vec3b>(yy2,xx2)[1];
		m2.at<double>(0,2) = img2.at<Vec3b>(yy2,xx2)[2];
		//cout << m2 << endl;
		for (int j=0;j<3;j++) {
			int k = 3*i+j;
			int m = 3*j;
			A.at<double>(k, m) = m1.at<double>(0, 0);
			A.at<double>(k, m+1) = m1.at<double>(0, 1);
			A.at<double>(k, m+2) = m1.at<double>(0, 2);
			A.at<double>(k, 9+j) = 1.0;
			b.at<double>(k, 0) = m2.at<double>(0, j);
		}
	}
	//cout << A << endl;
	cout << b << endl;
	solve(A, b, x, DECOMP_NORMAL);
	//cout << x << endl;
	Mat tmp = A*x-b;
	cout << tmp << endl;
	Mat T(4, 4, CV_32F, Scalar(0));
	T.at<float>(0,0) = (float)x.at<double>(0,0);
	T.at<float>(0,1) = (float)x.at<double>(1,0);
	T.at<float>(0,2) = (float)x.at<double>(2,0);
	T.at<float>(0,3) = (float)x.at<double>(3,0);
	T.at<float>(1,0) = (float)x.at<double>(4,0);
	T.at<float>(1,1) = (float)x.at<double>(5,0);
	T.at<float>(1,2) = (float)x.at<double>(6,0);
	T.at<float>(1,3) = (float)x.at<double>(7,0);
	T.at<float>(2,0) = (float)x.at<double>(8,0);
	T.at<float>(2,1) = (float)x.at<double>(9,0);
	T.at<float>(2,2) = (float)x.at<double>(10,0);
	T.at<float>(2,3) = (float)x.at<double>(11,0);
	T.at<float>(3,3) = 1.0f;
	transforms.push_back(T);
	//for (int j=0; j<(int)pixels1.size(); j++) {
	//	int x = (int)coordinates[j].x;
	//	int y = (int)coordinates[j].y;
	//	Mat color(4, 1, CV_32F);
	//	color.at<float>(0,0) = (float)pixels1[j].at<double>(0,0);//(float)tmp_img.at<Vec3b>(y,x)[0];////img1.at<Vec3b>(y,x)[0];
	//	color.at<float>(1,0) = (float)pixels1[j].at<double>(0,1);//(float)tmp_img.at<Vec3b>(y,x)[1];//
	//	color.at<float>(2,0) = (float)pixels1[j].at<double>(0,2);//(float)tmp_img.at<Vec3b>(y,x)[2];//
	//	color.at<float>(3,0) = 1.0f;
	//	Mat new_color = T*color;	
	//	img1.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(new_color.at<float>(0,0));
	//	img1.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(new_color.at<float>(1,0));
	//	img1.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(new_color.at<float>(2,0));
	//}
	return transforms;
}

/*
vector<Mat> Warper::AlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2)
{
	int n = ts1.size();
	vector<Mat> transforms;
	Mat C1, C2;
	Mat m1, m2;
	Mat evec1, evec2;
	Mat eval1, eval2;
	vector<Mat> pixels1, pixels2;
	vector<Point2d> coordinates;
	Mat tmp_img(img1);
	for (int i=0;i<n;i++) {
		float min_y1 = ts1[i].getMinY();
		float max_y1 = ts1[i].getMaxY();
		float min_y2 = ts2[i].getMinY();
		float max_y2 = ts2[i].getMaxY();
		float left, right;
		for (float y=min_y1; y<=max_y1; y+=1.0f) {
			FindLRBndry(y, ts1[i], &left, &right);
			for (float x=left; x<=right; x+=1.0) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img1.at<Vec3b>((int)y, (int)x)[0];
				p.at<double>(0,1) = img1.at<Vec3b>((int)y, (int)x)[1];
				p.at<double>(0,2) = img1.at<Vec3b>((int)y, (int)x)[2];
				pixels1.push_back(p);
				coordinates.push_back(Point2d(x, y));
			}
		}
		calcCovarMatrix(&pixels1[0], pixels1.size(), C1, m1, CV_COVAR_NORMAL);
		C1 = C1/(pixels1.size()-1);
		eigen(C1, eval1, evec1);
		//cout << evec1 << endl;
		//cout << eval1 << endl;
		for (float y=min_y2; y<=max_y2; y+=1.0f) {
			FindLRBndry(y, ts2[i], &left, &right);
			for (float x=left; x<=right; x+=1.0f) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img2.at<Vec3b>((int)y, (int)x)[0];
				p.at<double>(0,1) = img2.at<Vec3b>((int)y, (int)x)[1];
				p.at<double>(0,2) = img2.at<Vec3b>((int)y, (int)x)[2];
				pixels2.push_back(p);
			}
		}
		calcCovarMatrix(&pixels2[0], pixels2.size(), C2, m2, CV_COVAR_NORMAL); 
		C2 = C2/(pixels2.size()-1);
		eigen(C2, eval2, evec2);
		for (int j=0;j<3;j++) {
		double val = evec1.row(j).dot(evec2.row(j));
		if (val<0)
		evec2.row(j) = -evec2.row(j);
		}
		//cout << evec2 << endl;
		//cout << eval2 << endl;
		Mat vec1, vec2;
		vec1 = evec1.row(0).cross(evec1.row(1));
		vec2 = evec2.row(0).cross(evec2.row(1));
		if (vec1.dot(evec1.row(2))>0)
		evec1.row(2) = -evec1.row(2);
		if (vec2.dot(evec2.row(2))>0)
		evec2.row(2) = -evec2.row(2);
		Mat Tt1(4, 4, CV_32F, Scalar(0));
		Mat Tr1(4, 4, CV_32F, Scalar(0));
		Mat Ts1(4, 4, CV_32F, Scalar(0));
		Mat Tt2(4, 4, CV_32F, Scalar(0));
		Mat Tr2(4, 4, CV_32F, Scalar(0));
		Tt1.at<float>(0,0) = 1.0f;
		Tt1.at<float>(1,1) = 1.0f;
		Tt1.at<float>(2,2) = 1.0f;
		Tt1.at<float>(0,3) = -(float)m1.at<double>(0,0);
		Tt1.at<float>(1,3) = -(float)m1.at<double>(0,1);
		Tt1.at<float>(2,3) = -(float)m1.at<double>(0,2);
		Tt1.at<float>(3,3) = 1.0f;
		cout << "Tt1 = " << Tt1 << endl;
		Tt2.at<float>(0,0) = 1.0f;
		Tt2.at<float>(1,1) = 1.0f;
		Tt2.at<float>(2,2) = 1.0f;
		Tt2.at<float>(0,3) = (float)m2.at<double>(0,0);
		Tt2.at<float>(1,3) = (float)m2.at<double>(0,1);
		Tt2.at<float>(2,3) = (float)m2.at<double>(0,2);
		Tt2.at<float>(3,3) = 1.0f;
		cout << "Tt2 = " << Tt2 << endl;
		Tr1.at<float>(0,0) = (float)evec1.at<double>(0, 0);
		Tr1.at<float>(0,1) = (float)evec1.at<double>(0, 1);
		Tr1.at<float>(0,2) = (float)evec1.at<double>(0, 2);
		Tr1.at<float>(1,0) = (float)evec1.at<double>(1, 0);
		Tr1.at<float>(1,1) = (float)evec1.at<double>(1, 1);
		Tr1.at<float>(1,2) = (float)evec1.at<double>(1, 2);
		Tr1.at<float>(2,0) = (float)evec1.at<double>(2, 0);
		Tr1.at<float>(2,1) = (float)evec1.at<double>(2, 1);
		Tr1.at<float>(2,2) = (float)evec1.at<double>(2, 2);
		Tr1.at<float>(3,3) = 1.0f;
		cout << "Tr1 = " << Tr1 << endl;
		Tr2.at<float>(0,0) = (float)evec2.at<double>(0, 0);
		Tr2.at<float>(1,0) = (float)evec2.at<double>(0, 1);
		Tr2.at<float>(2,0) = (float)evec2.at<double>(0, 2);
		Tr2.at<float>(0,1) = (float)evec2.at<double>(1, 0);
		Tr2.at<float>(1,1) = (float)evec2.at<double>(1, 1);
		Tr2.at<float>(2,1) = (float)evec2.at<double>(1, 2);
		Tr2.at<float>(0,2) = (float)evec2.at<double>(2, 0);
		Tr2.at<float>(1,2) = (float)evec2.at<double>(2, 1);
		Tr2.at<float>(2,2) = (float)evec2.at<double>(2, 2);
		Tr2.at<float>(3,3) = 1.0f;
		cout << "Tr2 = " << Tr2 << endl;
		Ts1.at<float>(0,0) = (float)sqrt(eval2.at<double>(0, 0)/eval1.at<double>(0, 0));
		Ts1.at<float>(1,1) = (float)sqrt(eval2.at<double>(1, 0)/eval1.at<double>(1, 0));
		Ts1.at<float>(2,2) = (float)sqrt(eval2.at<double>(2, 0)/eval1.at<double>(2, 0));
		Ts1.at<float>(3,3) = 1.0f;
		cout << "Ts1 = " << Ts1 << endl;
		Mat T = Tt2*Tr2.t()*Ts1*Tr1*Tt1;
		cout << T << endl;
		transforms.push_back(T);
		//for (int j=0; j<(int)pixels1.size(); j++) {
		//	int x = (int)coordinates[j].x;
		//	int y = (int)coordinates[j].y;
		//	Mat color(4, 1, CV_32F);
		//	color.at<float>(0,0) = pixels1[j].at<double>(0,0);//(float)tmp_img.at<Vec3b>(y,x)[0];////img1.at<Vec3b>(y,x)[0];
		//	color.at<float>(1,0) = pixels1[j].at<double>(0,1);//(float)tmp_img.at<Vec3b>(y,x)[1];//
		//	color.at<float>(2,0) = pixels1[j].at<double>(0,2);//(float)tmp_img.at<Vec3b>(y,x)[2];//
		//	color.at<float>(3,0) = 1.0f;
		//	Mat new_color = T*color;	
		//	img1.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(new_color.at<float>(0,0));
		//	img1.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(new_color.at<float>(1,0));
		//	img1.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(new_color.at<float>(2,0));
		//}
		pixels1.clear();
		pixels2.clear();
		coordinates.clear();
	}
	return transforms;
}
*/

void Warper::WarpTriangle(Mat* src_img, Triangle & src, 
	Mat* dst_img, Triangle & dst, 
	bool blend)
{
	Mat affine_trans = FindAffine(dst, src);
	int min_y = (int)dst.getMinY();
	int max_y = (int)dst.getMaxY();
	float left, right;
	float src_x, src_y;
	for (int y=min_y; y<=max_y; y++) {
		FindLRBndry((float)y, dst, &left, &right);
		for (int x=(int)left; x<=(int)right; x++) {
			src_x = (float)(affine_trans.at<double>(0, 0)*x +
				affine_trans.at<double>(0, 1)*y +
				affine_trans.at<double>(0, 2));
			src_y = (float)(affine_trans.at<double>(1, 0)*x +
				affine_trans.at<double>(1, 1)*y +
				affine_trans.at<double>(1, 2));
			int diff = 0;
			for (int c=0; c<3; c++) 
				diff += abs(dst_img->at<Vec3b>(y, x)[c] - 
				src_img->at<Vec3b>((int)src_y, (int)src_x)[c]);
			double weight = exp(-alpha*diff);
			for (int c=0; c<3; c++) {
				if (diff>=0 && blend)
					dst_img->at<Vec3b>(y, x)[c] = (uchar)
					((1-weight)*dst_img->at<Vec3b>(y, x)[c] + 
					(weight)*src_img->at<Vec3b>((int)src_y, (int)src_x)[c]);
				else
					//dst_img->at<Vec3b>((int)y, (int)x)[c] = (uchar)
					//(weight*0.8*dst_img->at<Vec3b>((int)y, (int)x)[c] + 
					//(1-weight*0.8)*src_img->at<Vec3b>((int)src_y, (int)src_x)[c]);
					dst_img->at<Vec3b>(y, x)[c] = 
					src_img->at<Vec3b>((int)src_y, (int)src_x)[c]; 
			}
		}
	}
}

void Warper::WarpFace(Mat* src_img, vector<Triangle> *src_triangles, 
	Mat* dst_img, vector<Triangle> *dst_triangles,
	bool blend)
{
	int n_triangles = src_triangles->size();
	//AlignFace(Face *src_face, Face *dst_face);
	for (int i=0;i<n_triangles;i++) {
		WarpTriangle(src_img, (*src_triangles)[i], dst_img, (*dst_triangles)[i], blend);
	}
}

void Warper::TransformFC(FC fc, vector<Point2f>*srcv, vector<Point2f>*v, Mat T)
{
	int j=0;
	while (FC_Vertices[fc][j]!=0) {
		int i = FC_Vertices[fc][j]-1;
		float new_x, new_y;
		new_x = T.at<float>(0, 0)*(*srcv)[i].x + 
			T.at<float>(0, 1)*(*srcv)[i].y +
			T.at<float>(0, 2)*1.0f;
		new_y = T.at<float>(1, 0)*(*srcv)[i].x + 
			T.at<float>(1, 1)*(*srcv)[i].y +
			T.at<float>(1, 2)*1.0f;
		(*v)[i].x = new_x;
		(*v)[i].y = new_y;
		j++;
	}
}

void Warper::Transform(vector<Point2f> *v, Mat T)
{
	int n_v = v->size();
	float new_x, new_y;
	//	cout << "T : " << T << endl;
	for (int i=0;i<n_v;i++) {
		//		cout << "OLD : " << (*v)[i].x << " " << (*v)[i].y << endl;
		new_x = T.at<float>(0, 0)*(*v)[i].x + 
			T.at<float>(0, 1)*(*v)[i].y +
			T.at<float>(0, 2)*1.0f;
		new_y = T.at<float>(1, 0)*(*v)[i].x + 
			T.at<float>(1, 1)*(*v)[i].y +
			T.at<float>(1, 2)*1.0f;
		(*v)[i].x = new_x;
		(*v)[i].y = new_y;
		//		cout << "NEW : " <<  (*v)[i].x << " " << (*v)[i].y << endl;
	}
}

void Warper::CopyFCVertices(FC *fcs, int n, Face &f, vector<Point2f>& v)
{
	set<int> vertex_set;
	//vector<Point2f> vertices;
	for (int i=0;i<n;i++) {
		int j = 0;
		while (FC_Vertices[fcs[i]][j]!=0) {
			vertex_set.insert(FC_Vertices[fcs[i]][j]);
			j++;
		}
	}
	for (set<int>::iterator it=vertex_set.begin();it!=vertex_set.end();it++) {
		v[*it-1].x = f.base_vertices[*it-1].x;
		v[*it-1].y = f.base_vertices[*it-1].y;
	}
}

void Warper::SmoothBndry(Mat *img, Point2f p1, Point2f p2, int w)
{
	int neighbors = 2;
	float ymin = min(p1.y,p2.y);
	float ymax = max(p1.y,p2.y)-3;
	LineEquation l1(p1, p2);
	for (float y=ymin;y<=ymax;y+=1) {
		LineEquation l2(0.0f, 1, y);
		Point2f px = l1*l2;
		int left = (int)max((float)w, px.x-neighbors);
		int right = (int)min((float)img->cols-w-1, px.x+neighbors);
		for (int x=left; x<=right; x++) {
			int sum1=0, sum2=0, sum3=0;
			for (int xx=x-w;xx<=x+w;xx++) {
				sum1 += (*img).at<Vec3b>((int)px.y, xx)[0];
				sum2 += (*img).at<Vec3b>((int)px.y, xx)[1];
				sum3 += (*img).at<Vec3b>((int)px.y, xx)[2];
			}
			(*img).at<Vec3b>((int)px.y, x)[0] = sum1/(2*w+1);
			(*img).at<Vec3b>((int)px.y, x)[1] = sum2/(2*w+1);
			(*img).at<Vec3b>((int)px.y, x)[2] = sum3/(2*w+1);			
		}
	}
}

// Copy components of f1 to face f2
Face Warper::Surgery(Face& f1, Face& f2, bool affine)
{
	FC fc;
	vector<Point2f> tmp_vertices;
	tmp_vertices.resize(f1.base_vertices.size());
	copy(f2.base_vertices.begin(), f2.base_vertices.end(), tmp_vertices.begin());
	int n_groups = sizeof(AlignGroups)/sizeof(AlignGroups[0]);
	for (int i=0; i<n_groups; i++) {
		int n = 0;
		while (AlignGroups[i][n]!=(FC)-1)
			n++;
		CopyFCVertices(AlignGroups[i], n, f1, tmp_vertices);
		vector<Point2f> v1 = f1.getFCVertices(AlignGroups[i], n);
		vector<Point2f> v2 = f2.getFCVertices(AlignGroups[i], n);
		Mat S;
		if (affine)
			S = AffineAlign(&v1, &v2);
		else
			S = SimilarityAlign(&v1, &v2);
		for (int j=0; j<n;j++) {
			fc = AlignGroups[i][j];
			TransformFC(fc, &f1.base_vertices, &tmp_vertices, S);
		}
	}
	Face tmp_face(f2);
	tmp_face.setVertices(tmp_vertices);
	// Get the triangles from the temporary face object for final output
	vector<Triangle> ts0 = f1.getFCTriangles(FCS, sizeof(FCS)/sizeof(FCS[0]));
	vector<Triangle> ts1 = f2.getFCTriangles(FCS, sizeof(FCS)/sizeof(FCS[0]));
	vector<Triangle> ts2 = tmp_face.getFCTriangles(FCS, sizeof(FCS)/sizeof(FCS[0]));
	//// Draw triangles
	//f1.drawTriangles(ts0);
	//f2.drawTriangles(ts1);
	//tmp_face.drawTriangles(ts2);
	// Sample pixels from the source (face.base_img) to the target 
	// (tmp_face.base_img) using the aligned triangles. 
	// The background of the final output is from the target
	WarpFace(&f2.base_img, &ts1, &tmp_face.base_img, &ts2, false); // Tgt texture ==> aligned shape (over write tgt texture)
	WarpFace(&f1.base_img, &ts0, &tmp_face.base_img, &ts2, true); // Src texture ==> aligned shape (blend with tgt texture) 
	//WarpFace(&f1.base_img, &ts0, &tmp_face.base_img, &ts2, false); //Src texture ==> aligned shape (overwrite tgt texture)
	SmoothBndry(&tmp_face.base_img, tmp_face.base_vertices[13], tmp_face.base_vertices[12], 2); 
	SmoothBndry(&tmp_face.base_img, tmp_face.base_vertices[21], tmp_face.base_vertices[0], 2); 
	return tmp_face;
}

Mat Warper::ReColorSkin(Mat &T, vector<Triangle>ts, Mat* face_img)
{
	int n = ts.size();
	Mat tmp_img = face_img->clone();
	for (int i=0;i<n;i++) {
		int min_y1 = (int)ts[i].getMinY();
		int max_y1 = (int)ts[i].getMaxY();
		float left, right;
		for (int y=min_y1; y<=max_y1; y++) {
			FindLRBndry((float)y, ts[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++) {
				Mat color(4, 1, CV_32F);
				color.at<float>(0,0) = (float)face_img->at<Vec3b>(y,x)[0];
				color.at<float>(1,0) = (float)face_img->at<Vec3b>(y,x)[1];
				color.at<float>(2,0) = (float)face_img->at<Vec3b>(y,x)[2];
				color.at<float>(3,0) = 1.0f;
				Mat new_color = T*color;	
				tmp_img.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(new_color.at<float>(0,0));
				tmp_img.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(new_color.at<float>(1,0));
				tmp_img.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(new_color.at<float>(2,0));
			}
		}
	}
	return tmp_img;
}

void Warper::BuildSkinModel(vector<Triangle>ts, Mat* face_img, Mat &mean, Mat& cov) 
{
	int n = ts.size();
	vector<Mat> pixels;
	Mat tmp_img = face_img->clone();
	for (int i=0;i<n;i++) {
		int min_y1 = (int)ts[i].getMinY();
		int max_y1 = (int)ts[i].getMaxY();
		float left, right;
		for (int y=min_y1; y<=max_y1; y++) {
			FindLRBndry((float)y, ts[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = tmp_img.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = tmp_img.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = tmp_img.at<Vec3b>(y, x)[2];
				pixels.push_back(p);
			}
		}
	}
	calcCovarMatrix(&pixels[0], pixels.size(), cov, mean, CV_COVAR_NORMAL);
	cov = cov/(pixels.size()-1);
	skin_mean = mean.clone();
	skin_cov = cov.clone();
	//cout << mean << endl;
	//cout << cov << endl;
}

Mat Warper::ExtractSkin(Mat faceimg, Rect& roi, Mat& mean, Mat& cov, Mat& prob, double th)
{
	Mat mask(faceimg.rows, faceimg.cols, CV_8U, Scalar(0));
	for (int r=roi.y; r<roi.y+roi.height;r++) {
		for (int c=roi.x; c<roi.x+roi.width;c++) {
			double p1 = (double)faceimg.at<Vec3b>(r, c)[0];
			double p2 = (double)faceimg.at<Vec3b>(r, c)[1];
			double p3 = (double)faceimg.at<Vec3b>(r, c)[2];
			double d1 = (p1-mean.at<double>(0,0));
			double d2 = (p1-mean.at<double>(0,0));
			double d3 = (p1-mean.at<double>(0,0));
			double prob1 = exp(-d1*d1/2/cov.at<double>(0,0))/sqrt(cov.at<double>(0,0));
			double prob2 = exp(-d2*d2/2/cov.at<double>(1,1))/sqrt(cov.at<double>(1,1));
			double prob3 = exp(-d3*d3/2/cov.at<double>(2,2))/sqrt(cov.at<double>(2,2));
			prob.at<double>(r, c) = prob1*prob2*prob3;
			mask.at<uchar>(r,c) = prob.at<double>(r, c)>th?255:0;
		}
	}
	return mask;
}

Rect Warper::FindBoundary(vector<Triangle>ts)
{
	int minx=9999, maxx=0, miny=9999, maxy=0;
	for (int i=0;i<(int)ts.size();i++) {
		for (int j=0;j<3;j++) {
			if (ts[i].getVertex(j).x>maxx)
				maxx = (int)ts[i].getVertex(j).x;
			if (ts[i].getVertex(j).x<minx)
				minx = (int)ts[i].getVertex(j).x;
			if (ts[i].getVertex(j).y>maxy)
				maxy = (int)ts[i].getVertex(j).y;
			if (ts[i].getVertex(j).y<miny)
				miny = (int)ts[i].getVertex(j).y;
		}
	}
	return Rect(Point(minx,miny),Point(maxx,maxy));
}

void Warper::RemoveTrianglePixels(Mat& mask, vector<Triangle>ts)
{
	int n = ts.size();
	for (int i=0;i<n;i++) {
		int min_y1 = (int)ts[i].getMinY();
		int max_y1 = (int)ts[i].getMaxY();
		float left, right;
		for (int y=min_y1; y<=max_y1; y++) {
			FindLRBndry((float)y, ts[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++)
				mask.at<uchar>(y,x) = 0;
		}
	}
}

Mat Warper::DetectForehead(Face& face, double skin_th)
{
	Mat mean(1, 3, CV_64F);
	Mat cov(3, 3, CV_64F);
	FC fcg[][5] = {{ALL}, {SKIN}, {LEYE, REYE, NOSE, MOUTH}}; //人臉五官群組表
	vector<Triangle> skin_tris1 = face.getFCTriangles(fcg[2], 4);
	BuildSkinModel(skin_tris1, &face.base_img, mean, cov);
	Mat prob(face.base_img.rows, face.base_img.cols, CV_64F, Scalar(0.0));
	skin_tris1 = face.getFCTriangles(fcg[0], 4);
	Mat tri_img = face.drawTriangles(skin_tris1);
	//namedWindow("Triangles");
	//imshow("Triangles", tri_img);
	Rect face_bndry = FindBoundary(skin_tris1);
	Rect forehead_roi(face_bndry.x, max(face_bndry.y-face_bndry.height/3,0), 
					  face_bndry.width, 2*face_bndry.height/3);
	Mat forehead_mask = ExtractSkin(face.base_img, forehead_roi, mean, cov, prob, skin_th);
	RemoveTrianglePixels(forehead_mask, skin_tris1);
	return forehead_mask;
}

Mat Warper::TrianglesToMask(vector<Triangle> ts, int rows, int cols)
{
	int n = ts.size();
	Mat mask(rows, cols, CV_8U, Scalar(0));
	for (int i=0;i<n;i++) {
		int min_y1 = (int)ts[i].getMinY();
		int max_y1 = (int)ts[i].getMaxY();
		float left, right;
		for (int y=min_y1; y<=max_y1; y++) {
			FindLRBndry((float)y, ts[i], &left, &right);
			for (int x=(int)left; x<=(int)right; x++)
				mask.at<uchar>(y,x) = 255;
		}
	}
	return mask;
}

Mat Warper::WeightMask(Mat mask, int type, int neighbor)
{
	Mat out(mask.rows, mask.cols, CV_8U, Scalar(0));
	Mat edge, tmp_mask;
	double sigma = 20;
	Size sz(neighbor, neighbor);
	//Canny(mask, edge, 250, 255);
	if (type==0) //Distance Transform 
	{
		distanceTransform(mask, edge, CV_DIST_L1, 3);
		normalize(edge, edge, 0, 255, NORM_MINMAX);
		for (int i=0;i<(int)edge.rows;i++) {
			for (int j=0;j<(int)edge.cols;j++) {
				out.at<uchar>(i,j) = (uchar)(edge.at<float>(i,j));
			}
		}
	}
	else
		GaussianBlur(mask, out, sz, sigma);
	return out;
}

Mat Warper::GetFaceMask(Face& face)
{
	Mat fh_mask = DetectForehead(face);
	FC fc = ALL;
	vector<Triangle> ts = face.getFCTriangles(&fc, 1);
	Mat f_mask = TrianglesToMask(ts, face.base_img.rows, face.base_img.cols);
	Mat final_mask;
	bitwise_or(fh_mask, f_mask, final_mask);
	return final_mask;
}

Mat Warper::ReColorMaskedSkin(Mat &T, Mat& mask, Mat* face_img)
{
	Mat tmp_img = face_img->clone();
	Mat color(4, 1, CV_32F), new_color;
	for (int i=0;i<(int)face_img->rows;i++) {
		for (int j=0;j<(int)face_img->cols;j++) {
			if (mask.at<uchar>(i,j)!=0) {
				float w = (float)(mask.at<uchar>(i,j))/255.0f;
				color.at<float>(0,0) = (float)face_img->at<Vec3b>(i,j)[0];
				color.at<float>(1,0) = (float)face_img->at<Vec3b>(i,j)[1];
				color.at<float>(2,0) = (float)face_img->at<Vec3b>(i,j)[2];
				color.at<float>(3,0) = 1.0f;
				new_color = T*color;
				uchar b = saturate_cast<uchar>(new_color.at<float>(0,0));
				uchar g = saturate_cast<uchar>(new_color.at<float>(1,0));
				uchar r = saturate_cast<uchar>(new_color.at<float>(2,0));
				tmp_img.at<Vec3b>(i,j)[0] = (uchar)(w*b+(1-w)*face_img->at<Vec3b>(i,j)[0]);
				tmp_img.at<Vec3b>(i,j)[1] = (uchar)(w*g+(1-w)*face_img->at<Vec3b>(i,j)[1]);
				tmp_img.at<Vec3b>(i,j)[2] = (uchar)(w*r+(1-w)*face_img->at<Vec3b>(i,j)[2]);
			}
		}
	}
	return tmp_img;
}

Mat Warper::AlignMaskedSkinColor(Mat& img1, Mat mask1, Mat& img2, Mat mask2)
{
	Mat C1, C2;
	Mat m1, m2;
	int step = 3;
	vector<Mat> pixels1, pixels2;
	Mat tmp_img = img1.clone();
	for (int y=0;y<img1.rows;y+=step) { //y+=step speeds up the process (step>1)
		for (int x=0;x<img1.cols;x+=step) { //x+=step speeds up the process (step>1)
			if (mask1.at<uchar>(y,x)!=0) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img1.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = img1.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = img1.at<Vec3b>(y, x)[2];
				pixels1.push_back(p);
			}
		}
	}
	calcCovarMatrix(&pixels1[0], pixels1.size(), C1, m1, CV_COVAR_NORMAL);
	C1 = C1/(pixels1.size()-1);
	cout << m1 << endl;
	cout << C1 << endl;
	for (int y=0;y<img2.rows;y+=step) {//y+=step speeds up the process
		for (int x=0;x<img2.cols;x+=step) {//x+=step speeds up the process
			if (mask2.at<uchar>(y,x)!=0) {
				Mat p(1, 3, CV_64F);
				p.at<double>(0,0) = img2.at<Vec3b>(y, x)[0];
				p.at<double>(0,1) = img2.at<Vec3b>(y, x)[1];
				p.at<double>(0,2) = img2.at<Vec3b>(y, x)[2];
				pixels2.push_back(p);
			}
		}
	}
	calcCovarMatrix(&pixels2[0], pixels2.size(), C2, m2, CV_COVAR_NORMAL);
	C2 = C2/(pixels2.size()-1);

	Mat Tt1(4, 4, CV_32F, Scalar(0));
	Mat Ts1(4, 4, CV_32F, Scalar(0));
	Mat Tt2(4, 4, CV_32F, Scalar(0));
	Tt1.at<float>(0,0) = 1.0f;
	Tt1.at<float>(1,1) = 1.0f;
	Tt1.at<float>(2,2) = 1.0f;
	Tt1.at<float>(0,3) = -(float)m1.at<double>(0,0);
	Tt1.at<float>(1,3) = -(float)m1.at<double>(0,1);
	Tt1.at<float>(2,3) = -(float)m1.at<double>(0,2);
	Tt1.at<float>(3,3) = 1.0f;
	Tt2.at<float>(0,0) = 1.0f;
	Tt2.at<float>(1,1) = 1.0f;
	Tt2.at<float>(2,2) = 1.0f;
	Tt2.at<float>(0,3) = (float)m2.at<double>(0,0);
	Tt2.at<float>(1,3) = (float)m2.at<double>(0,1);
	Tt2.at<float>(2,3) = (float)m2.at<double>(0,2);
	Tt2.at<float>(3,3) = 1.0f;
	Ts1.at<float>(0,0) = (float)sqrt(C2.at<double>(0, 0)/C1.at<double>(0, 0));
	Ts1.at<float>(1,1) = (float)sqrt(C2.at<double>(1, 0)/C1.at<double>(1, 0));
	Ts1.at<float>(2,2) = (float)sqrt(C2.at<double>(2, 0)/C1.at<double>(2, 0));
	Ts1.at<float>(3,3) = 1.0f;
	Mat T = Tt2*Ts1*Tt1;
	//cout << Tt2 << endl;
	//cout << Ts1 << endl;
	//cout << Tt1 << endl;
	//cout << T << endl;
	return T;
}

Mat Warper::AdaptFaceTone(Face& face, Face& ref_face)
{
	Mat out;
	Mat mask, ref_mask;
	FC fcg[][5] = {{BETWEENEYES},{ALL},{BETWEENEYES,LCHEEKBONE,RCHEEKBONE}};
	//vector<Triangle> srcts = src_face.getFCTriangles(fcg[0], 1);
	mask = GetFaceMask(face);
	vector<Triangle> refts = ref_face.getFCTriangles(fcg[1], 1);
	ref_mask = TrianglesToMask(refts, ref_face.base_img.rows, ref_face.base_img.cols);
	Mat T = AlignMaskedSkinColor(face.base_img, mask, ref_face.base_img, ref_mask);
	vector<Triangle> dstts = face.getFCTriangles(fcg[0],1);
	RemoveTrianglePixels(mask, dstts); //排出眉心區域的recolor以免改變部分人留海的髮色，會讓合成結果不自然
	Rect roi = FindBoundary(dstts);
	Mat prob(face.base_img.rows, face.base_img.cols, CV_64F, Scalar(0.0));
	Mat skin_mask = ExtractSkin(face.base_img, roi, skin_mean, skin_cov, prob, 0.000001);
	bitwise_or(mask, skin_mask, mask); //再納入眉心區域的膚色像素，以達到將非膚色區域完全排除於recolor之外
	Mat wmask = WeightMask(mask, 1, 5);
	//namedWindow("wmask");
	//imshow("wmask", wmask);
	out = ReColorMaskedSkin(T, wmask, &face.base_img);
	return out;
}