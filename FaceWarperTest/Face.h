#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include "Triangle.h"
#define NO_OF_VERTICES	64 //���I�ƥ�
enum FC {NOSE, LEYE, REYE, MOUTH, JAW, SJAW, LBROW, RBROW, SKIN, ALL, LCHEEK, RCHEEK, LEYELID, REYELID}; //�w�q�Ӥ��x���쪺�N��

using namespace std;
using namespace cv;

class Face //��ܤ@�ӤH�y���������O
{
public:
	Face(void);
	Face(Point2f *v, string img_file); //��J64�ӳ��I�M�v���ɦW�A�i�غc�@�ӤH�y����
	Face(Point2f *v, Mat* face_img);
	Face(Face& f); //�Q�Τ@�ӤH�y����ƻs�غc�t�@�ӤH�y
	virtual ~Face(void);
	void setVertex(int n, Point2f v); //�]�w��n�ӳ��I�y��
	void composeTriangle(); //�q64�ӳ��I���ͩҦ��զ��H�y���T����
	void readImg(string filename); //�q�ɮ�Ū�����H�y���󪺤H�y�v��
	Mat drawTriangles(vector<Triangle>& tris); //�b�H�y���󪺳ƥ��v��(tri_img)�W�e�X�Ҧ��T����
	int noOfTriangles(); //�Ǧ^���H�y����T�����`��
	int noOfVertices(); //�Ǧ^���H�y���󪺳��I�`��
	void setVertices(vector<Point2f> new_vertices){ //�Q�Τ@�ճ��I��s���N�{�����H�y���I
		copy(new_vertices.begin(), new_vertices.end(), 
			base_vertices.begin());
		composeTriangle(); //��s��n���s���ͩҦ��T����
	}
	vector<Triangle> getFCTriangles(FC* fcs, int n); //���o�]�t���w���x����(�i�H�@�өΦh��)���T����
	vector<Point2f> getFCVertices(FC* fcs, int n); //���o�]�t���w���x����(�i�H�@�өΦh��)�����I
	vector<Point2f> base_vertices; //�Ҧ����I
	vector<Triangle> triangles; //�Ҧ��T����
	Mat base_img; //��ø�s�T���檺�ƥ��H�y�v��
	Mat tri_img; //�H�y�v��
};

