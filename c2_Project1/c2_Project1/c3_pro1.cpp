#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

void main() {
	string path = "D:/c++/OpenCv/chapter1/Project1/Project1/resources/guy.jpg";
	Mat img = imread(path);
	Mat imgResize, imgCrop;
	//cout << img.size() << endl;
	
	resize(img, imgResize, Size(), 0.5, 0.5);
	Rect roi(200, 100, 300, 100);
	imgCrop = img(roi);
	imshow("Image", img);
	imshow("Image Resize", imgResize);
	imshow("Image Crop", imgCrop);

	waitKey(0);
}