#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc.hpp"
#include <math.h>

using namespace std;
using namespace cv;

//全局变量

int zoomTrackbarValue;							//缩小的值
String windowHybridImg = "output image3";		//输出窗口标题
Mat dstfinal;									//最后输出的图像

//缩小函数
void handleZoom()
{
	Mat zoomImg = dstfinal.clone();
	for (int i = 0; i < zoomTrackbarValue; i++)
	{
		pyrDown(zoomImg, zoomImg);
	}
	imshow(windowHybridImg, zoomImg);
	resizeWindow(windowHybridImg, dstfinal.size());
}

//主函数
int main()
{
	//加载第一张图片

	Mat src, dst;
	src = imread("C:\\Users\\joker\\Desktop\\test\\cat.bmp");

	if (!src.data)
	{
		cout << "could not found image" << endl;
		return 0;
	}
	//高斯平滑

	//Mat kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	//filter2D(src, dst, -1, kernel);
	GaussianBlur(src, dst, cv::Size(5, 5), 0, 0, BORDER_DEFAULT);
	namedWindow("output image1", WINDOW_AUTOSIZE);
	imshow("output image1", dst);

	//加载第二张图片

	Mat src1, dst1;
	src1 = imread("C:\\Users\\joker\\Desktop\\test\\dog.bmp");
	if (!src1.data)
	{
		cout << "could not found image" << endl;
		return 0;
	}
	//高斯平滑

	GaussianBlur(src1, dst1, cv::Size(5, 5), 0, 0, BORDER_DEFAULT);
	//Mat src1_gray, abs_dst1;
	//cvtColor(src1, src1_gray, COLOR_BGR2GRAY);
	//Laplacian(src1_gray, abs_dst1, CV_8UC3, 3, 1, 0, BORDER_DEFAULT);
	//convertScaleAbs(abs_dst1, dst1);
	//Mat kernel = (Mat_<char>(3, 3) << 1, 0, -1, 2, 0, -2, 1, 0, -1);
	//filter2D(src1, dst1, -1, kernel);
	dst1 = src1 - dst1 + src1;
	namedWindow("output image2", WINDOW_AUTOSIZE);
	imshow("output image2", dst1);

	//线性相加到dstfinal中，各占0.5权重
	dstfinal = Mat(dst1.size(), dst1.type());
	dstfinal = Scalar(0, 0, 0);
	addWeighted(dst, 0.5, dst1, 0.5, 0, dstfinal);

	namedWindow(windowHybridImg, WINDOW_AUTOSIZE);
	imshow(windowHybridImg, dstfinal);
	//调用缩小滑动选项
	createTrackbar("Zoom:", windowHybridImg, &zoomTrackbarValue, 4, TrackbarCallback(handleZoom));

	waitKey(0);
	return 0;
}