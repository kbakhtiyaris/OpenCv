#include <opencv2/imgcodecs.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

void main(){

	string path = "D:/c++/OpenCv/Project2/Project2/resources/test_video.mp4";
	VideoCapture cap(path);
	Mat img;
	//cap.open(0); // for webcam
	while (true)
	{
		
		cap.read(img);

		imshow("Image", img);
		waitKey(20);

	}

}