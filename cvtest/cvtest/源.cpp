#include <iostream>
//#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc.hpp"
#include <math.h>

using namespace std;
using namespace cv;

/*int main()
{
	Mat src, dst;
	src = imread("C:\\Users\\joker\\Desktop\\test\\yinhui.jpg");

	if (!src.data)
	{
		cout << "could not found image" << endl;
		return 0;
	}

	//namedWindow("input image", WINDOW_AUTOSIZE);
	//imshow("input image", src);

	Mat kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	filter2D(src, dst, -1, kernel);
	namedWindow("output image1", WINDOW_AUTOSIZE);
	imshow("output image1", dst);

	Mat src1, dst1;
	src1 = imread("C:\\Users\\joker\\Desktop\\test\\jiexika.jpg");
	if (!src1.data)
	{
		cout << "could not found image" << endl;
		return 0;
	}
	GaussianBlur(src1, dst1, cv::Size(3, 3), 0, 0, BORDER_DEFAULT);
	//Mat src1_gray, abs_dst1;
	//cvtColor(src1, src1_gray, COLOR_BGR2GRAY);
	//Laplacian(src1_gray, abs_dst1, CV_16S, 3, 1, 0, BORDER_DEFAULT);
	//convertScaleAbs(abs_dst1, dst1);
	namedWindow("output image2", WINDOW_AUTOSIZE);
	imshow("output image2", dst1);

	Mat dstfinal = Mat(dst1.size(), dst1.type());
	dstfinal = Scalar(0, 0, 0);
	addWeighted(dst, 0.5, dst1, 0.5, 0, dstfinal);
	namedWindow("output image3", WINDOW_AUTOSIZE);
	imshow("output image3", dstfinal);

	waitKey(0);
	return 0;
}*/

// Methods
char getSelectionFromUser();
void createAndShowHybridImage();
void handleHybrid();
void handleZoom();

// Global variables
const int PYR_LEVEL = 8;
char selection;
String windowHybridImg = "Hybrid image (result)";

Mat sourceImg1, sourceImg2, hybridImg, zoomImg;
Mat img1Gaussian[PYR_LEVEL];
Mat img1Laplacian[PYR_LEVEL - 1];
Mat img2Gaussian[PYR_LEVEL];
Mat img2Laplacian[PYR_LEVEL - 1];

int hybridTrackbarValue = 0;
int zoomTrackbarValue;
const int MAX_ZOOM = min(PYR_LEVEL, 4);
int main()
{
	sourceImg1 = imread("C:\\Users\\joker\\Desktop\\test\\cat.bmp");
	sourceImg2 = imread("C:\\Users\\joker\\Desktop\\test\\dog.bmp");
	if (sourceImg1.empty() || sourceImg2.empty())
	{
		cout << "Error while loading image/s" << endl;
		return -1;
	}
	// Makes the images on the same size if needed
	if (sourceImg1.size < sourceImg2.size)
		resize(sourceImg2, sourceImg2, sourceImg1.size());
	else if (sourceImg2.size < sourceImg1.size)
		resize(sourceImg1, sourceImg1, sourceImg2.size());

	cvtColor(sourceImg1, sourceImg1, COLOR_BGR2GRAY);
	cvtColor(sourceImg2, sourceImg2, COLOR_BGR2GRAY);
	// Clone source images into gaussian arrays
	img1Gaussian[0] = sourceImg1.clone();
	img2Gaussian[0] = sourceImg2.clone();

	// Create the Gaussian and Laplacian pyramids (arrays)
	for (int i = 0; i < PYR_LEVEL - 1; i++)
	{
		// Image 1 arrays.
		pyrDown(img1Gaussian[i], img1Gaussian[i + 1]);
		pyrUp(img1Gaussian[i + 1], img1Gaussian[i + 1], img1Gaussian[i].size());
		img1Laplacian[i] = img1Gaussian[i] - img1Gaussian[i + 1];

		// Image 2 arrays.
		pyrDown(img2Gaussian[i], img2Gaussian[i + 1]);
		pyrUp(img2Gaussian[i + 1], img2Gaussian[i + 1], img2Gaussian[i].size());
		img2Laplacian[i] = img2Gaussian[i] - img2Gaussian[i + 1];
	}

	// Get selection from user
	selection = getSelectionFromUser();

	// Create windows for source images and show them
	// imshow auto create default window with WINDOW_AUTOSIZE parameter if does not exist.
	imshow("image 1", sourceImg1);
	imshow("image 2", sourceImg2);

	// Prepare the hybrid (result) window for user interaction.
	createAndShowHybridImage();

	waitKey(0);
	return 0;
} // End main

char getSelectionFromUser() {
	cout << "###############################################" << endl;
	cout << "#     Hybrid Image Program   -   jokergss     #" << endl;
	cout << "###############################################" << endl;

	selection = ' ';

	do
	{
		cout << "Please select one option from the menu:" << endl;
		cout << "1) The first image is closer" << endl;
		cout << "2) The second image is closer\n" << endl;
		cin >> selection;
	} while (selection != '1' && selection != '2');

	return selection;
}

void createAndShowHybridImage() {
	// Show the selected image (original without hybrid)
	hybridImg = selection == '1' ? sourceImg1.clone() : sourceImg2.clone();
	imshow(windowHybridImg, hybridImg);

	// Create Hybrid and zoom trackbar to hybrid window
	createTrackbar("Hybrid:", windowHybridImg, &hybridTrackbarValue, PYR_LEVEL - 1, TrackbarCallback(handleHybrid));
	createTrackbar("Zoom:", windowHybridImg, &zoomTrackbarValue, MAX_ZOOM, TrackbarCallback(handleZoom));
}

// Invoke each time when user changes the hybrid value
void handleHybrid() {
	// Choose the correct image (Gaussian) according to user selection
	hybridImg = selection == '1' ? img1Gaussian[hybridTrackbarValue].clone() : img2Gaussian[hybridTrackbarValue].clone();

	// Add the Laplasians Image/s according to user selection
	for (int i = 0; i < hybridTrackbarValue; i++) {
		hybridImg += selection == '1' ? img2Laplacian[i] : img1Laplacian[i];
		/*if (selection == '1')
		{
			addWeighted(hybridImg, 0.5, img2Gaussian[i], 0.5, 0, hybridImg);
		}
		else
		{
			addWeighted(hybridImg, 0.5, img1Gaussian[i], 0.5, 0, hybridImg);
		}*/
	}

	// In order to save the zoom level before changes the hybrid level
	handleZoom();
}

// Invoke each time when user changes the zoom value
void handleZoom() {
	zoomImg = hybridImg.clone();

	for (int i = 0; i < zoomTrackbarValue; i++)
	{
		pyrDown(zoomImg, zoomImg);
	}

	imshow(windowHybridImg, zoomImg);
	resizeWindow(windowHybridImg, hybridImg.size());
}