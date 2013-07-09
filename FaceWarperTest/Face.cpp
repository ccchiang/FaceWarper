#include "StdAfx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "Face.h"
int TriTable[][3] = { //定義每個三角格包含的頂點編號
24,	23,	54, //第一個三角格
26,	22,	27, //第二個三角格
26,	27,	28, //後面的依此類推 ...
9,	46,	10,
55,	25,	24,
26,	28,	25,
23,	29,	52,
23,	24,	29,
59,	44,	58,
23,	52,	53,
2,	1,	57,
58,	57,	56,
56,	26,	25,
50,	51,	41,
43,	42,	58,
6,	50,	7,
51,	40,	41,
11,	46,	12,
42,	50,	41,
48,	47,	9,
63,	62,	53,
38,	27,	22,
18,	20,	19,
44,	48,	43,
17,	16,	21,
28,	29,	24,
38,	37,	27,
28,	36,	29,
36,	35,	29,
37,	28,	27,
37,	36,	28,
35,	52,	29,
30,	35,	36,
30,	64,	35,
58,	55,	54,
56,	1,	26,
23,	53,	54,
52,	35,	64,
54,	53,	62,
53,	52,	64,
39,	22,	1,
39,	38,	22,
55,	56,	25,
1,	22,	26,
55,	24,	54,
25,	28,	24,
40,	2,	57,
4,	3,	40,
4,	2,	3,
5,	40,	51,
3,	2,	40,
5,	4,	40,
6,	5,	51,
42,	41,	57,
40,	57,	41,
7,	50,	49,
6,	51,	50,
44,	43,	58,
49,	50,	43,
58,	56,	55,
57,	1,	56,
58,	42,	57,
43,	50,	42,
62,	61,	58,
45,	48,	44,
15,	62,	63,
58,	54,	62,
46,	59,	12,
60,	58,	61,
60,	59,	58,
45,	44,	59,
49,	48,	8,
49,	43,	48,
46,	45,	59,
46,	47,	45,
9,	47,	46,
48,	45,	47,
49,	8,	7,
48,	9,	8,
46,	11,	10,
17,	60,	61,
12,	59,	60,
60,	17,	18,
61,	62,	16,
60,	13,	12,
60,	18,	13,
17,	21,	20,
18,	14,	13,
14,	34,	13,
19,	20,	32,
64,	63,	53,
16,	17,	61,
64,	15,	63,
64,	30,	21,
15,	16,	62,
20,	31,	32,
31,	21,	30,
15,	21,	16,
15,	64,	21,
17,	20,	18,
21,	31,	20,
14,	19,	33,
33,	19,	32,
14,	18,	19,
14,	33,	34
};

int FC_Vertices[10][64] = { //定義組成不同五官群的所有頂點查表,順序與個數(目前訂了十個)要與 enum FC 的定義一致
	{52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64},	//0. NOSE
	{14, 15, 16, 17, 18, 19, 20, 21},						//1. LEYE
	{22, 23, 24, 25, 26, 27, 28, 29},						//2. REYE
	{40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51},		//3. MOUTH
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},			//4. JAW
	{1, 2, 5, 6, 7, 8, 9, 12, 13},							//5. SJAW (subjaw 比JAW少了幾個頂點)
	{30, 31, 32, 33, 34},									//6. LBROW (左眉毛)
	{35, 36, 37, 38, 39},									//7. RBROW
	{14, 15, 16, 17, 18, 21, 22, 23, 24, 25, 26,			//8. SKIN (臉頰和下巴部位，不包含眼、嘴)
	 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,	
	 40, 46, 47, 48, 49, 50, 51,
	 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
	{ 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,				//9. ALL(全部)
	 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
	 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
	 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 
	 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
	 61, 62, 63, 64}};

Face::Face(void)
{
	base_vertices.resize(NO_OF_VERTICES);
}

Face::Face(Face& f)
{
	base_img = f.base_img.clone();
	tri_img = f.tri_img.clone();
	base_vertices.resize(f.base_vertices.size());
	copy(f.base_vertices.begin(), f.base_vertices.end(),
		base_vertices.begin());
	composeTriangle();
}

Face::Face(Point2f *v, string img_file)
{
	for (int i=0;i<NO_OF_VERTICES;i++) 
		base_vertices.push_back(v[i]);
	base_img = imread(img_file);
	this->composeTriangle();
}

Face::Face(Point2f *v, Mat* img)
{
	for (int i=0;i<NO_OF_VERTICES;i++) 
		base_vertices.push_back(v[i]);
	base_img = img->clone();
	this->composeTriangle();
}

Face::~Face(void)
{
}

vector<Point2f> Face::getFCVertices(FC* fcs, int n)
{
	set<int> vertex_set;
	vector<Point2f> vertices;
	for (int i=0;i<n;i++) {
		int j = 0;
		while (FC_Vertices[fcs[i]][j]!=0) {
			vertex_set.insert(FC_Vertices[fcs[i]][j]);
			j++;
		}
	}
	for (set<int>::iterator it=vertex_set.begin();it!=vertex_set.end();it++) {
		vertices.push_back(Point2f(base_vertices[*it-1].x, base_vertices[*it-1].y));
	}
	return vertices;
}

vector<Triangle> Face::getFCTriangles(FC* fcs, int n)
{
	vector<Triangle> tri;
	set<int> vertex_set;
	for (int i=0;i<n;i++) {
		int j = 0;
		while (FC_Vertices[fcs[i]][j]!=0) {
			vertex_set.insert(FC_Vertices[fcs[i]][j]);
			j++;
		}
	}
	int n_tri = sizeof(TriTable)/sizeof(int)/3;
	for (int i=0;i<n_tri;i++) {
		bool flag[3]={false, false, false};
		for (set<int>::iterator it=vertex_set.begin();it!=vertex_set.end();it++) {
			flag[0] = flag[0] | (*it==TriTable[i][0]);
			flag[1] = flag[1] | (*it==TriTable[i][1]);
			flag[2] = flag[2] | (*it==TriTable[i][2]);
			if (flag[0]&&flag[1]&&flag[2]) {
				tri.push_back(triangles[i]);
				break;
			}
		}
	}
	return tri;
}

int Face::noOfTriangles()
{
	return sizeof(TriTable)/sizeof(int)/3;
}

int Face::noOfVertices()
{
	return base_vertices.size();
}

void Face::setVertex(int n, Point2f v)
{
	base_vertices[n].x = v.x;
	base_vertices[n].y = v.y;
}

void Face::composeTriangle()
{
	int n_triangles = noOfTriangles();
	triangles.clear();
	for (int i=0;i<n_triangles;i++) {
		Triangle t(base_vertices[TriTable[i][0]-1],
				   base_vertices[TriTable[i][1]-1],
				   base_vertices[TriTable[i][2]-1]); 
		triangles.push_back(t);
	}
}

void Face::readImg(string filename)
{
	base_img = imread(filename);
}

Mat Face::drawTriangles(vector<Triangle>& tris)
{
	int n_triangles = tris.size();
	tri_img = base_img.clone();
	for (int i=0;i<n_triangles;i++) {
		line(tri_img, tris[i].getVertex(0), 
					   tris[i].getVertex(1),
					   Scalar(0, 0, 255));
		line(tri_img, tris[i].getVertex(1), 
					   tris[i].getVertex(2),
					   Scalar(0, 0, 255));
		line(tri_img, tris[i].getVertex(0), 
					   tris[i].getVertex(2),
					   Scalar(0, 0, 255));
	}
	return tri_img;
}