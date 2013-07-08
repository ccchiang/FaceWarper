#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#define INF	999999999.0f

using namespace std;
using namespace cv;
class LineEquation //定義一個ax+by=c的直線方程式類別
{
public:
	float a; //係數a
	float b; //係數b
	float c; //係數c
	LineEquation(float a1, float b1, float c1){a = a1; b = b1; c = c1;} //建構子，輸入三係數做初始化
	LineEquation(Point2f p1, Point2f p2)//利用二座標點建構一個線性方程式，自動算出三係數
	{
		a = p2.y-p1.y;
		b = p1.x-p2.x;
		c = p1.x*p2.y-p2.x*p1.y;
	}
	float getA(){return a;}//傳回係數a
	float getB(){return b;}//傳回係數b
	float getC(){return c;}//傳回係數c
	Point2f operator *(LineEquation l) { //定義計算二直線交點的運算子(*) 例如：11*l2傳回一個交點座標
		Point2f p;
		float denum = b*l.getA()-a*l.getB();
		if (denum==0) p.x = p.y = INF;
		else {
			p.x = (b*l.getC()-c*l.getB())/denum;
			p.y = (c*l.getA()-a*l.getC())/denum;
		}
		return p;
	}
};

class Triangle //用來表示人臉一個三角格之類別
{
public:
	Triangle(void);
	// Construct a triangle with three vertices, 
	// arranging them into counterclockwise order
	Triangle(Point2f v1, Point2f v2, Point2f v3){ //用三頂點建構一個三角形
		v.push_back(v1);
		v.push_back(v2);
		v.push_back(v3);
	}
	// Find the top Y position of the triangle
	float getMinY(){return min(v[0].y,min(v[1].y, v[2].y));}
	// Find the bottom Y position of the triangle
	float getMaxY(){return max(v[0].y,max(v[1].y, v[2].y));}
	virtual ~Triangle(void);
	// Get vertex v[n] (0<=n<=2)
	Point2f& getVertex(int n) {return v[n];}
	// Set the vertex with a 2D point p
	void setVertex(int n, Point2f p){v[n].x = p.x;v[n].y=p.y;}
	// Get the side connecting the vi and vj (0<=i,j<=2, i =/= j)
	LineEquation getSide(int i, int j){
		LineEquation l(v[i], v[j]);
		return l;
	}
	// Check if the given line intersect with the side connecting v1 and v2
	bool Intersect(LineEquation l, int vid1, int vid2, Point2f* p) {
		LineEquation l1 = getSide(vid1, vid2);
		Point2f q = l*l1;
		p->x = q.x;
		p->y = q.y;
		float left = min(v[vid1].x,v[vid2].x);
		float right = max(v[vid1].x,v[vid2].x);
		float top = min(v[vid1].y,v[vid2].y);
		float bottom = max(v[vid1].y,v[vid2].y);
		if (q.x>=left && q.x<=right && //檢查交點有否落在直線上
			q.y>=top && q.y<=bottom)
			return true;
		else
			return false;
	}
	
private:
	vector<Point2f> v; //三頂點
	vector<int> id; //三頂點編號(根據淙淮編號)
};

