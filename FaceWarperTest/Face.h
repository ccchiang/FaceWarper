#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include "Triangle.h"
#define NO_OF_VERTICES	64 //頂點數目
enum FC {NOSE, LEYE, REYE, MOUTH, JAW, SJAW, LBROW, RBROW, SKIN, ALL, LCHEEK, RCHEEK, LEYELID, REYELID}; //定義個五官部位的代號

using namespace std;
using namespace cv;

class Face //表示一個人臉的物件類別
{
public:
	Face(void);
	Face(Point2f *v, string img_file); //輸入64個頂點和影像檔名，可建構一個人臉物件
	Face(Point2f *v, Mat* face_img);
	Face(Face& f); //利用一個人臉物件複製建構另一個人臉
	virtual ~Face(void);
	void setVertex(int n, Point2f v); //設定第n個頂點座標
	void composeTriangle(); //從64個頂點產生所有組成人臉的三角格
	void readImg(string filename); //從檔案讀取此人臉物件的人臉影像
	Mat drawTriangles(vector<Triangle>& tris); //在人臉物件的備份影像(tri_img)上畫出所有三角格
	int noOfTriangles(); //傳回此人臉物件三角格總數
	int noOfVertices(); //傳回此人臉物件的頂點總數
	void setVertices(vector<Point2f> new_vertices){ //利用一組頂點更新替代現有的人臉頂點
		copy(new_vertices.begin(), new_vertices.end(), 
			base_vertices.begin());
		composeTriangle(); //更新後要重新產生所有三角格
	}
	vector<Triangle> getFCTriangles(FC* fcs, int n); //取得包含指定五官部位(可以一個或多個)的三角格
	vector<Point2f> getFCVertices(FC* fcs, int n); //取得包含指定五官部位(可以一個或多個)的頂點
	vector<Point2f> base_vertices; //所有頂點
	vector<Triangle> triangles; //所有三角格
	Mat base_img; //供繪製三角格的備份人臉影像
	Mat tri_img; //人臉影像
};

