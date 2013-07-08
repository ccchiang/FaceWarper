// FaceWarperTest.cpp : 定義主控台應用程式的進入點。
//
#include <opencv2\core\core.hpp>
#include <opencv2\legacy\legacy.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\nonfree\features2d.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <iostream>
#include <fstream>
#include "Triangle.h"
#include "Warper.h"
#include "Face.h"
#include "FDetector.h"

using namespace std;
using namespace cv;

extern FC FCS[5];

string convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

int main(int argc, char ** argv)
{
	Warper w(0.0175);
/* Test the line intersection function
	Point2f pp1 = Point2f(0.0f, 0.0f);
	Point2f pp2 = Point2f(1.0f, 0.0f);
	LineEquation l(pp1, pp2);
	LineEquation l1(-4.0f,-3.0f,-1.0f);
	Point2f p = l*l1;
*/

/* Test the triangle-to-triangle image wapring
	Triangle t1(Point2f(0, 0), Point2f(1, 0), Point2f(0, 1));
	Triangle t2(Point2f(0, 0), Point2f(1, 0.5), Point2f(-0.5, 1));
	Mat m = w.FindAffine(t1, t2);
	cout << m << endl;
	Mat P(3, 3, CV_64F);
	P.at<double>(0, 0) = t1.getVertex(0).x;
	P.at<double>(0, 1) = t1.getVertex(0).y;
	P.at<double>(0, 2) = 1.0f;
	P.at<double>(1, 0) = t1.getVertex(1).x;
	P.at<double>(1, 1) = t1.getVertex(1).y;
	P.at<double>(1, 2) = 1.0f;
	P.at<double>(2, 0) = t1.getVertex(2).x;
	P.at<double>(2, 1) = t1.getVertex(2).y;
	P.at<double>(2, 2) = 1.0f;
	cout << P << endl;
	Mat res = m*P.t();
	cout << res << endl;
	Mat src_img = imread("Koala.jpg");
	Mat dst_img = imread("Penguins.jpg");
	Point2f p1(100, 100), p2(800, 200), p3(200,700);
	Point2f p4(100, 50), p5(400, 100), p6(250, 500);
	Triangle t3(p1, p2, p3);
	Triangle t4(p4, p5, p6);
	w.WarpTriangle(&src_img, t3, &dst_img, t4, false); //做兩三角型之間的形變
	line(src_img, p1, p2, Scalar(0,0,255),2);
	line(src_img, p2, p3, Scalar(0,0,255),2);
	line(src_img, p1, p3, Scalar(0,0,255),2);
	line(dst_img, p4, p5, Scalar(0,0,255),2);
	line(dst_img, p5, p6, Scalar(0,0,255),2);
	line(dst_img, p4, p6, Scalar(0,0,255),2);
	namedWindow("Src");
	imshow("Src", src_img);
	namedWindow("Dst");
	imshow("Dst", dst_img);
*/	

/* Test the color alignment

	Point2f p1(135, 100), p2(90, 30), p3(180, 50), p4(360, 60);
	Point2f p5(135, 100), p6(90, 30), p7(180, 50), p8(360, 60);
	Triangle t1(p1, p2, p3), t3(p2, p3, p4),t5(p1,p2,p4);
	Triangle t2(p5, p6, p7), t4(p6, p7, p8),t6(p5,p6,p8);
	vector<Triangle> ts1(1, t1); //準備一個三角形陣列
	ts1.push_back(t3);ts1.push_back(t5);
	vector<Triangle> ts2(1, t2); //準備另一個三角形陣列
	ts2.push_back(t4);ts2.push_back(t6);
	Mat src_img = imread("Koala.jpg");
	Mat dst_img = imread("Penguins.jpg");

	w.AlignColor(src_img, ts1, dst_img, ts2);//利用兩三角格陣列所包含的像素估測修正轉換並直接調整src_img的像素
	namedWindow("Src Color");
	imshow("Src Color", src_img);
	namedWindow("Dst Color");
	imshow("Dst Color", dst_img);
*/


/* Testing Facial Feature Detection and Alignment 
	string src0 = "d001"; //來源人臉檔名
	string dst0 = "mean"; //來源人臉檔名
	Mat img = imread(src0+".jpg");
	Mat rimg = imread(dst0+".jpg");
	FDetector fd;
	MatchResult mr = fd.RegisterFaces(img, rimg);
	vector<Point2f> srcpts;
	vector<Point2f> dstpts;
	for (int i=0;i<(int)mr.qkps.size();i++){
		srcpts.push_back(Point2f(mr.qkps[i].pt));
		dstpts.push_back(Point2f(mr.tkps[i].pt));
	}
	Mat aft = w.AffineAlign(&srcpts, &dstpts);
	Mat raft = w.AffineAlign(&dstpts, &srcpts);
	vector<Point2f> tmpsrc(srcpts);
	vector<Point2f> tmp;
	tmp.push_back(Point2f(0, 0));
	tmp.push_back(Point2f((float)img.cols, 0));
	tmp.push_back(Point2f(0, (float)img.rows));
	tmp.push_back(Point2f((float)img.cols, (float)img.rows));
	w.Transform(&tmp, aft);
	int minx=10000, maxx=-1, miny=10000, maxy=-1;
	for (int i=0;i<(int)tmp.size();i++) {
		if (tmp[i].x<minx) minx = (int)tmp[i].x;
		if (tmp[i].y<miny) miny = (int)tmp[i].y;
		if (tmp[i].x>maxx) maxx = (int)tmp[i].x;
		if (tmp[i].y>maxy) maxy = (int)tmp[i].y;
	}
	Mat aligned_img(maxy-miny+1, maxx-minx+1,CV_8UC3);
	for (int r=0;r<aligned_img.rows;r++) {
		for (int c=0;c<aligned_img.cols;c++) {
			Mat p(3, 1, CV_32F);
			p.at<float>(0,0) = (float)(c+minx);
			p.at<float>(1,0) = (float)(r+miny); 
			p.at<float>(2,0) = 1;
			Mat xy = raft*p;
			int x = (int)xy.at<float>(0,0);
			int y = (int)xy.at<float>(1,0);
			if (x<0||y<0||x>=img.cols||y>=img.rows) {
				aligned_img.at<Vec3b>(r, c)[0] = 255;
				aligned_img.at<Vec3b>(r, c)[1] = 255;
				aligned_img.at<Vec3b>(r, c)[2] = 255;
			}
			else {
				aligned_img.at<Vec3b>(r, c)[0] = img.at<Vec3b>(y, x)[0];
				aligned_img.at<Vec3b>(r, c)[1] = img.at<Vec3b>(y, x)[1];
				aligned_img.at<Vec3b>(r, c)[2] = img.at<Vec3b>(y, x)[2];
			}
		}
	}
	cout << aft << endl;
	cout << raft << endl;
	namedWindow("Aligned");
	imshow("Aligned", aligned_img);
*/


/* Testing the face surgery
*/
	ifstream ifs;
	string src = "c001"; //來源人臉檔名
	string dst = "d001"; //目的人臉檔名
	ifs.open(dst+".txt"); //讀入目的人臉頂點座標檔案(一個人臉)
	Point2f v1[NO_OF_VERTICES], v2[NO_OF_VERTICES];
	for (int i=0;i<NO_OF_VERTICES;i++) {
		ifs >> v1[i].x >> v1[i].y;
	}
	ifs.close();
	// Construct the target face object
	Face dst_face(v1, dst+".jpg"); //讀入目的人臉影像，並產生人臉物件

	// Read the vertices of the source face from txt file 
	ifs.open(src+".txt"); //讀入來源人臉頂點座標(一個人臉)
	for (int i=0;i<NO_OF_VERTICES;i++) {
		ifs >> v2[i].x >> v2[i].y;
	}
	ifs.close();
	// Construct the source face object
	Face src_face(v2, src+".jpg"); //讀入來源人臉影像
	FC fcg[][5] = {{ALL}, {SKIN}, {LEYE, REYE, NOSE, MOUTH}}; //人臉五官群組表
	vector<Triangle> skin_tris1 = src_face.getFCTriangles(fcg[2], 4); //先指定用來做膚色調整的來源人臉三角格陣列
	vector<Triangle> skin_tris2 = dst_face.getFCTriangles(fcg[2], 4); //再指定用來做膚色調整的目的人臉三角格陣列
	vector<Mat> Ts = w.NewAlignColor(src_face.base_img, skin_tris1, dst_face.base_img, skin_tris2); //利用上面二組三角格陣列中的三角格所包含的像素估測膚色，並直接調整來源人臉的膚色
	cout << Ts[0] << endl;
	vector<Triangle> all_tris = src_face.getFCTriangles(fcg[0], 1); //再指定用來做膚色調整的目的人臉三角格陣列
	Mat new_skin_img = w.ReColorSkin(Ts[0], all_tris, &src_face.base_img);
	Face new_src_face(v2, &new_skin_img);
	bool affine = false; //指定人臉對齊要用affine transform或是similarity transform
	Face out_face = w.Surgery(new_src_face, dst_face, affine); //進行虛擬整形
	int new_width = src_face.base_img.cols*2/3;
	int new_height = src_face.base_img.rows*2/3;
	namedWindow("Recolored Src");
	moveWindow("Recolored Src", new_width, new_height);
	Mat t0_img;
	resize(new_src_face.base_img, t0_img, Size(new_width, new_height));
	imshow("Recolored Src", t0_img);
	namedWindow("Src");
	moveWindow("Src", 0, new_height);
	Mat t1_img;
	resize(src_face.base_img, t1_img, Size(new_width, new_height));
	imshow("Src", t1_img);
//	namedWindow("TriSrc");
//	src_face.drawTriangles(src_face.getFCTriangles(FCS, sizeof(FCS)/sizeof(FCS[0])));
//	imshow("TriSrc", src_face.tri_img);
	namedWindow("Dst");
	moveWindow("Dst", 0, 0);
	Mat t2_img;
	resize(dst_face.base_img, t2_img, Size(new_width, new_height));
	imshow("Dst", t2_img);
	//namedWindow("TriDst");
	//imshow("TriDst", dst_face.tri_img);
	namedWindow("Out");
	moveWindow("Out", new_width, 0);
	Mat t3_img;
	resize(out_face.base_img, t3_img, Size(new_width, new_height));
	imshow("Out", t3_img);
	//namedWindow("TriOut");
	//imshow("TriOut", out_face.tri_img);


/* Generating a mean face texture 
	ifstream ifs;
	string src = "c001"; //來源人臉檔名
	string dst = "mean"; //目的人臉檔名
	ifs.open(dst+".txt"); //讀入目的人臉頂點座標檔案(一個人臉)
	Point2f v1[NO_OF_VERTICES], v2[NO_OF_VERTICES];
	for (int i=0;i<NO_OF_VERTICES;i++) {
		ifs >> v1[i].x >> v1[i].y;
	}
	ifs.close();
	Face dst_face(v1, dst+".jpg"); //讀入目的人臉影像，並產生人臉物件

	// Read the vertices of the source face from txt file 
	ifs.open(src+".txt"); //讀入來源人臉頂點座標(一個人臉)
	for (int i=0;i<NO_OF_VERTICES;i++) {
		ifs >> v2[i].x >> v2[i].y;
	}
	ifs.close();
	// Construct the source face object
	Face src_face(v2, src+".jpg"); //讀入來源人臉影像
	FC all_parts = ALL;
	vector<Triangle> srcts = src_face.getFCTriangles(&all_parts, 1);
	vector<Triangle> dstts = dst_face.getFCTriangles(&all_parts, 1);
	w.WarpFace(&src_face.base_img, &srcts, &dst_face.base_img, &dstts, false);
	namedWindow("Mean Face");
	imshow("Mean Face", dst_face.base_img);
	namedWindow("Src Face");
	imshow("Src Face", src_face.base_img);
*/

/* Testing feature detector

	string src = "c001"; //來源人臉檔名
	string dst = "mean"; //目的人臉檔名
	Mat srcface = imread(src+".jpg");
	Mat dstface = imread(dst+".jpg");
	FDetector fd;
	vector<Rect> face1 = fd.DetectFacialFeature(srcface, 0, NULL);
	vector<Rect> face2 = fd.DetectFacialFeature(dstface, 0, NULL);
	SURF gftdetector(100);//(50, 0.01, 1.0, 2, true, 0.01);
	vector<KeyPoint> kpts1, kpts2;
	Mat t1 = srcface(face1[0]);
	Mat t2 = dstface(face2[0]);
	gftdetector.detect(t1, kpts1);
	gftdetector.detect(t2, kpts2);
	Mat out1, out2;
	drawKeypoints(t1, kpts1, out1);
	drawKeypoints(t2, kpts2, out2);
	namedWindow("Src KPs");
	namedWindow("Dst KPs");
	imshow("Src KPs", out1);
	imshow("Dst KPs", out2);
*/

	waitKey();


	return 0;
}