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
class Warper //�n�ΨӰ��H�y�ܧΪ��������O
{
public:
	Warper(void);
	Warper(double a):alpha(a){};
	virtual ~Warper(void);
private:
	void FindLRBndry(float k, Triangle &dst, float* left, float* right); //��Xy=k��{���P�T����dst�ۥ檺���B�k��ɤ�x��m(�bWarpTriangle���n�Ψ�)
public:
	double alpha;
	void SetAlpha(double a){alpha = a;}
	Mat FindAffine(Triangle & t1, Triangle & t2); //��X��T���涡����g�ഫ T*t1==>t2
	void WarpTriangle(Mat* src_img, Triangle & src, //�Ndst�T���檺�v���Ԧ��Psrc�T����@�˪��Ϊ� 
						  Mat* dst_img, Triangle & dst, 
						  bool blend); //true:��X�n�V�Jsrc���v���Afalse:��X�����u��dst���v���ө�
	void WarpFace(Mat* src_img, vector<Triangle> *src_triangles, //��i�y�Ҧ��T����v��������
					  Mat* dst_img, vector<Triangle> *dst_triangles,
					  bool blend);
	Mat SimilarityAlign(vector<Point2f> *src_v, vector<Point2f> *dst_v);//�G�i�H�y�������(�ϥ�similarity transform)
	Mat AffineAlign(vector<Point2f> *src_v, vector<Point2f> *dst_v); //�G�i�H�y�������(�ϥ�affine transform)
	void Transform(vector<Point2f> *v, Mat T); //�NT�ഫ�M�Φbv�̩Ҧ������I�W�o��s���I��m(�s��m������s�bv�W)
	void TransformFC(FC fc, vector<Point2f>* srcv, vector<Point2f>*v, Mat T); //�NT�ഫ�M�Φb���w�����x�����I��m�A���G��bv
	Face Surgery(Face &f1, Face &f2, bool affine); //���H�y���x��� (�bf2�W�X���Xf1���Ϊ�, �v���Ӧ�f2, �Ϊ��Ӧ�f1)
	void SmoothBndry(Mat *img, Point2f p1, Point2f p2, int w); //�����䲪(���w�G�I�Φ��@�u�q�A�b�u���u�q������)
	void CopyFCVertices(FC *fcs, int n, Face& f, vector<Point2f>& v); //�������w�����x�s��(�i���u�@��)�Ҧ������I��v
	vector<Mat> AlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2);//����ץ�, �Q��img1�Wts1�Ҧ��T����]�t�������Pimg2
																							   //�Wts2�Ҧ��T���檺����������ץ��������A�o��@���ഫ�x�}T
																							   //�Nimg1�W�������C��M�WT��i�o�Pimg2�W�C��۪��C�� T*c1 ==> c2
	vector<Mat> NewAlignColor(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2);//����ץ�, �Q��img1�Wts1�Ҧ��T����]�t�������Pimg2
																							   //�Wts2�Ҧ��T���檺����������ץ��������A�o��@���ഫ�x�}T
																							   //�Nimg1�W�������C��M�WT��i�o�Pimg2�W�C��۪��C�� T*c1 ==> c2
	vector<Mat> AlignColorLMS(Mat& img1, vector<Triangle> &ts1, Mat& img2, vector<Triangle> &ts2);
	Mat ReColorSkin(Mat &T, vector<Triangle>ts, Mat* face_img); //�Q�Φ�m�ഫ�x�}�ഫface_img���T����}�C�Ҳ[�\�ϰ쪺������m�æ^�Ǥ@�ӷs���H�y�v��
};

