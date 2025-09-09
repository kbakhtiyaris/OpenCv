#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <sstream>

#include <ctime>    // for time functions
#include <iomanip>  // for put_time

using namespace cv;
using namespace std;

Mat imgOriginal, imgGray, imgCanny, imgThre, imgWarp, imgCrop;
vector<Point> initialPoints, docPoints;
float w = 420, h = 596;
int maxArea;

Mat preProcessing(Mat img)
{
	Mat imgGray, imgBlur, imgCanny, imgDil, imgThre;
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	//erode(imgThre, imgDil, kernel);
	//erode(imgThre, imgThre, kernel);
	return imgDil;
}

vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest;
	
	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);
		cout << area << endl;
		string objectType;
		if (area > 1000) {
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			if (area > maxArea && conPoly[i].size()==4) 
			{
				//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);
				biggest = { conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3] };
				maxArea = area;
			}


			//rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
			int objCor = (int)conPoly[i].size();
		}
	}

	return biggest;

}

void drawPoints(vector<Point> points, Scalar color) 
{
	for (int i = 0; i < points.size(); i++) 
	{
		circle(imgOriginal, points[i], 5, color, FILLED);
		putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 2, color, 2);
	}
}
vector<Point> reorder(vector<Point> points) 
{
	vector<Point> newPoints;
	vector<int> sumPoints, subPoints;
	for (int i = 0; i < 4; i++) 
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}
	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // 0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); // 1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); // 2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // 3
	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));
	return imgWarp;
}

// to save with unique name


string getTimeStamp() {
    auto t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);   // ✅ safe MSVC version

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}




int main()
{
	
	string path = "resources/doc.jpg";
	imgOriginal = imread(path);
	//resize(imgOriginal, imgOriginal, Size(), 1, 1);

	// preprocessing
	imgThre = preProcessing(imgOriginal);
	//get contours- biggest
	initialPoints = getContours(imgThre);
	//drawPoints(initialPoints, Scalar(0, 0, 255));
	docPoints = reorder(initialPoints);
	//drawPoints(docPoints, Scalar(0, 255, 0));
	
	//wrap

	imgWarp = getWarp(imgOriginal, docPoints, w, h);

	//crop
	int cropVal = 10;
	Rect roi(cropVal, cropVal, w - 10, h - 10);
	imgCrop = imgWarp(roi);
	//rectangle(imgWarp, Point(5, 5), Point(w - 5, h - 5), Scalar(0, 255, 0), 10);
	//display
	imshow("Image", imgOriginal);
	//imshow("Image dilation", imgThre);
	//imshow("Image Warp", imgWarp);
	imshow("Image Crop", imgCrop);
	// 📂 Folder where you want to save
	string folder = "C:/Users/kbakhtiyar/Documents/document tester/";

	// generate timestamp
	string timestamp = getTimeStamp();

	// build filenames
	//string filenameWarp = folder + "warped_doc_" + timestamp + ".jpg";
	string filenameCrop = folder + "cropped_doc_" + timestamp + ".jpg";

	// save images
	//imwrite(filenameWarp, imgWarp);
	imwrite(filenameCrop, imgCrop);

	//cout << "✅ Saved: " << filenameWarp << endl;
	cout << " Saved: " << filenameCrop << endl;
	waitKey(0);

}