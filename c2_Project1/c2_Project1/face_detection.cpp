#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

using namespace cv;
using namespace std;
//Mat imgGray, imgBlur, imgCanny, imgDil;
Mat img, imgGray;

void main()
{


	string path = "D:/c++/OpenCv/chapter1/Project1/Project1/resources/guy.jpg";
	Mat img = imread(path);
	
	CascadeClassifier faceCascade;
	faceCascade.load("D:/c++/fine/opencv/data/haarcascades/haarcascade_frontalface_default.xml");
	cvtColor(img, imgGray, COLOR_BGR2GRAY);

	if (faceCascade.empty()) { cout << "XML file not loaded" << endl;  }

	vector<Rect> faces;
	faceCascade.detectMultiScale(imgGray, faces, 1.1, 10);
	for (int i = 0; i < faces.size(); i++) 
	{
		rectangle(img, faces[i].tl(), faces[i].br(), Scalar(255, 0, 255), 3);
	}

	imshow("Image", imgGray);
	waitKey(0);

}