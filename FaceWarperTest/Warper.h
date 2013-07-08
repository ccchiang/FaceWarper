#pragma once
#include <iostream>
#include <fstream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Triangle.h"
#include "Face.h"

using namespace cv;
using namespace std;
class Warper //要用來做人臉變形的物件類別
{
public:
	Warper(void);
	Warper(double a):alpha(a){};
	virtual ~Warper(void);
private:
	void FindLRBndry(float k, Triangle &dst, float* left, float* right); //找出y=k方程式與三角形dst相交的左、右邊界之x位置(在WarpTriangle中要用到)
public:
	double alpha;
	void SetAlpha(double a){alpha = a;}
	Mat FindAffine(Triangle & t1, Triangle & t2); //找出兩三角格間的仿射轉換 T*t1==>t2
	void WarpTriangle(Mat* src_img, Triangle & src, //將dst三角格的影像拉成與src三角格一樣的形狀 
						  Mat* dst_img, Triangle & dst, 
						  bool blend); //true:輸出要混入src的影像，false:輸出完全只用dst的影像來拉
	void WarpFace(Mat* src_img, vector<Triangle> *src_triangles, //整張臉所有三角格影像的形變
					  Mat* dst_img, vector<Triangle> *dst_triangles,
					  bool blend);
	Mat SimilarityAlign(vector<Point2f> *src_v, vector<Point2f> *dst_v);//二張人臉間的對齊(使用similarity transform)
	Mat AffineAlign(vector<Point2f> *src_v, vector<Point2f> *dst_v); //二張人臉間的對齊(使用affine transform)
	void Transform(vector<Point2f> *v, Mat T); //將T轉換套用在v裡所有的頂點上得到新頂點位置(新位置直接更新在v上)
	void TransformFC(FC fc, vector<Point2f>* srcv, vector<Point2f>*v, Mat T); //將T轉換套用在指定的五官之頂點位置，結果放在v
	Face Surgery(Face &f1, Face &f2, bool affine); //做人臉五官整形 (在f2上合成出f1的形狀, 影像來自f2, 形狀來自f1)
	void SmoothBndry(Mat *img, Point2f p1, Point2f p2, int w); //消除邊痕(指定二點形成一線段，在沿此線段做除痕)
	void CopyFCVertices(FC *fcs, int n, Face& f, vector<Point2f>& v); //拷貝指定的五官群組(可不只一個)所有的頂點到v
	vector<Mat> AlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2);//膚色修正, 利用img1上ts1所有三角格包含的像素與img2
																							   //上ts2所有三角格的像素做膚色修正之估測，得到一個轉換矩陣T
																							   //將img1上的像素顏色套上T後可得與img2上顏色相近的顏色 T*c1 ==> c2
	vector<Mat> NewAlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2);//膚色修正, 利用img1上ts1所有三角格包含的像素與img2
																							   //上ts2所有三角格的像素做膚色修正之估測，得到一個轉換矩陣T
																							   //將img1上的像素顏色套上T後可得與img2上顏色相近的顏色 T*c1 ==> c2
	vector<Mat> AlignColorLMS(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2);
	Mat ReColorSkin(Mat &T, vector<Triangle>ts, Mat* face_img); //利用色彩轉換矩陣轉換face_img中三角格陣列所涵蓋區域的像素色彩並回傳一個新的人臉影像
};

