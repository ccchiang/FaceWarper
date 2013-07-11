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

class FaceDresser
{
public:
	FaceDresser(void);
	~FaceDresser(void);
	Mat Smoother(Mat& face, Mat& mask);
};

