#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

using namespace cv;
using namespace std;
//Mat imgGray, imgBlur, imgCanny, imgDil;


void main() {


	string path = "D:/c++/OpenCv/chapter1/Project1/Project1/resources/guy.jpg";
	Mat img = imread(path);
	CascadeClassifier faceCascade;
	faceCascade.load("D:/c++/OpenCv/c2_Project1/c2_Project1/haarcascade_frontalface_default");
	if (faceCascade.empty()) { cout << "XML file not loaded" << endl; return; }
	imshow("Image", img);
	waitKey(0);

}