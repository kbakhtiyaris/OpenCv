#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;
Mat matrix, imgWrap;
void main() {

	
	string path = "D:/c++/OpenCv/c2_Project1/c2_Project1/cards.jpg";
	Mat img = imread(path);
	Point2f src[4] = { {87,59}, {156,40}, {181,66}, {107,90} };
	Point2f dst[4] = { {0,0}, {300,0}, {300,300}, {0,300} };

	matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWrap, matrix, Size(300, 300));
	for (int i = 0; i < 4; i++) {
		circle(img, src[i], 10, Scalar(0, 0, 255), FILLED);
	}
	

	imshow("Image", img);
	imshow("Image Wrap", imgWrap);

	waitKey(0);

}