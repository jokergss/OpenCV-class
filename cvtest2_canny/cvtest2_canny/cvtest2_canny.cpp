#include "head.h"

Mat imageOriginal = imread("C:/Users/joker/Desktop/test/sikadi.jpg");	//初始图片
Mat imageGray;					//灰度转换后图片
Mat imageGaussion;				//高斯模糊后图片
Mat imageGradientY;				//Y方向差分值
Mat imageGradientX;				//X方向差分值
Mat imageGradient;				//梯度计算
Mat imageNMS;					//非极大值抑制
Mat imageLowThreshold;			//弱边缘
Mat imageHighThreshold;			//强边缘
Mat imageCanny;					//canny算法图像
Mat imageResult;				//输出结果图像
int lowThreshold = 70;			//弱边缘
int highThreshold = 40;			//强边缘

int main() {
	namedWindow(WINDOW_NAME);
	createTrackbar("Low Thres", WINDOW_NAME, &lowThreshold, 100, onParaChange);
	createTrackbar("High Thres", WINDOW_NAME, &highThreshold, 100, onParaChange);
	onParaChange(0, nullptr);
	waitKey(0);
	return 0;
}

Mat Canny(const Mat &frame, int lowThreshold, int highThreshold, int kernelSize = 3) {
	//    转换为灰度图像
	cvtColor(frame, imageGray, CV_BGR2GRAY);
	//    高斯滤波去除噪音
	GaussianBlur(imageGray, imageGaussion, Size(kernelSize, kernelSize), 0, 0);
	//    增强
	double *pointDirection; //定义梯度方向角数组
	GenerateGradient(imageGaussion, imageGradientX, imageGradientY, pointDirection);  //计算X、Y方向梯度和方向角
	CombineGradient(imageGradientX, imageGradientY, imageGradient);   //计算X、Y方向梯度融合幅值
	//    检测
	NMS(imageGradient, imageNMS, pointDirection);  //局部非极大值抑制
	SplitWithThreshold(imageNMS, imageLowThreshold, imageHighThreshold, lowThreshold, highThreshold);        //双阈值处理
	LinkEdge(imageCanny, imageLowThreshold, imageHighThreshold);					//连接边缘

	return imageCanny;
}

void GenerateGradient(const Mat &imageSource, Mat &imageSobelX, Mat &imageSobelY, double *&pointDirection) {
	pointDirection = new double[(imageSource.rows - 1) * (imageSource.cols - 1)];

	imageSobelX = Mat::zeros(imageSource.size(), CV_32SC1);
	imageSobelY = Mat::zeros(imageSource.size(), CV_32SC1);

	int step = imageSource.step;
	int stepXY = imageSobelX.step;
	int rowCount = imageSource.rows;
	int columnCount = imageSource.cols;

	for (int i = 1; i < (rowCount - 1); i++) {
		const uchar *pixelsPreviousRow = imageSource.ptr<uchar>(i - 1);
		const uchar *pixelsThisRow = imageSource.ptr<uchar>(i);
		const uchar *pixelsNextRow = imageSource.ptr<uchar>(i + 1);
		uchar *pixelsThisRow_x = imageSobelX.ptr<uchar>(i);
		uchar *pixelsThisRow_y = imageSobelY.ptr<uchar>(i);
		for (int j = 1, k = 0; j < (columnCount - 1); j++, k++) {
			//通过指针遍历图像上每一个像素
			double gradY = pixelsPreviousRow[j + 1] + pixelsThisRow[j + 1] * 2 + pixelsNextRow[j + 1] -
				pixelsPreviousRow[j - 1] - pixelsThisRow[j - 1] * 2 - pixelsNextRow[j - 1];
			double gradX = pixelsNextRow[j - 1] + pixelsNextRow[j] * 2 + pixelsNextRow[j + 1] -
				pixelsPreviousRow[j - 1] - pixelsPreviousRow[j] * 2 - pixelsPreviousRow[j + 1];
			pixelsThisRow_x[j * (stepXY / step)] = static_cast<uchar>(abs(gradX));
			pixelsThisRow_y[j * (stepXY / step)] = static_cast<uchar>(abs(gradY));
			if (gradX != 0) {
				pointDirection[k] = atan(gradY / gradX) * 57.3 + 90;// (- PI / 2, PI / 2)转换到(0, 180)
			}
			else {
				pointDirection[k] = 180;
			}
		}
	}

	convertScaleAbs(imageSobelX, imageSobelX);
	convertScaleAbs(imageSobelY, imageSobelY);
}

void CombineGradient(const Mat &imageGradX, const Mat &imageGradY, Mat &SobelAmpXY) {
	SobelAmpXY = Mat::zeros(imageGradX.size(), CV_32FC1);
	for (int i = 0; i < SobelAmpXY.rows; i++) {
		const uchar *pixelsThisRow_x = imageGradX.ptr<uchar>(i);
		const uchar *pixelsThisRow_y = imageGradY.ptr<uchar>(i);
		float *pixelsThisRow_xy = SobelAmpXY.ptr<float>(i);
		for (int j = 0; j < SobelAmpXY.cols; j++) {
			const uchar xj = pixelsThisRow_x[j];
			const uchar yj = pixelsThisRow_y[j];
			pixelsThisRow_xy[j] = static_cast<float>(sqrt(xj * xj + yj * yj));
		}
	}
	convertScaleAbs(SobelAmpXY, SobelAmpXY);
}

void NMS(const Mat &imageInput, Mat &imageOutput, double *pointDirection) {
	imageOutput = imageInput.clone();
	int rowCount = imageInput.rows;
	int columnCount = imageInput.cols;
	for (int i = 1; i < rowCount - 1; i++) {
		uchar *pixelsPreviousRow = imageOutput.ptr<uchar>(i - 1);
		uchar *pixelsThisRow = imageOutput.ptr<uchar>(i);
		uchar *pixelsNextRow = imageOutput.ptr<uchar>(i + 1);
		for (int j = 1, k = 0; j < columnCount - 1; j++, k++) {
			double tPD = tan(pointDirection[i * (columnCount - 1) + j]);
			double tPD_180 = tan(180 - pointDirection[i * (columnCount - 1) + j]);

			if (pointDirection[k] <= 45) {
				if (pixelsThisRow[j] <=
					(pixelsThisRow[j + 1] + (pixelsPreviousRow[j + 1] - pixelsThisRow[j + 1]) * tPD) ||
					(pixelsThisRow[j] <=
					(pixelsThisRow[j - 1] + (pixelsNextRow[j - 1] - pixelsThisRow[j - 1]) * tPD))) {
					pixelsThisRow[j] = 0;
				}
			}
			else if (pointDirection[k] <= 90) {
				if (pixelsThisRow[j] <=
					(pixelsPreviousRow[j] + (pixelsPreviousRow[j + 1] - pixelsPreviousRow[j]) / tPD) ||
					pixelsThisRow[j] <= (pixelsNextRow[j] + (pixelsNextRow[j - 1] - pixelsNextRow[j]) / tPD)) {
					pixelsThisRow[j] = 0;
				}
			}
			else if (pointDirection[k] <= 135) {
				if (pixelsThisRow[j] <=
					(pixelsPreviousRow[j] + (pixelsPreviousRow[j - 1] - pixelsPreviousRow[j]) / tPD_180) ||
					pixelsThisRow[j] <= (pixelsNextRow[j] + (pixelsNextRow[j + 1] - pixelsNextRow[j]) / tPD_180)) {
					pixelsThisRow[j] = 0;
				}
			}
			else if (pointDirection[k] <= 180) {
				if (pixelsThisRow[j] <=
					(pixelsThisRow[j - 1] + (pixelsPreviousRow[j - 1] - pixelsThisRow[j - 1]) * tPD_180) ||
					pixelsThisRow[j] <= (pixelsThisRow[j + 1] + (pixelsNextRow[j + 1] - pixelsThisRow[j]) * tPD_180)) {
					pixelsThisRow[j] = 0;
				}
			}
			else {
				cout << "Invalid pointDirection: " << pointDirection[k] << endl;
			}
		}
	}
}

void SplitWithThreshold(const Mat &imageInput, Mat &lowOutput, Mat &highOutput, double lowThreshold, double highThreshold) {
	lowOutput = imageInput.clone();
	highOutput = imageInput.clone();
	int rowCount = imageInput.rows;
	int columnCount = imageInput.cols;
	for (int i = 0; i < rowCount; i++) {
		const uchar *pixelsThisRow = imageInput.ptr<uchar>(i);
		uchar *pixelsThisRow_low = lowOutput.ptr<uchar>(i);
		uchar *pixelsThisRow_high = highOutput.ptr<uchar>(i);
		for (int j = 0; j < columnCount; j++) {
			uchar pixel = pixelsThisRow[j];
			if (pixel >= highThreshold) {
				pixelsThisRow_high[j] = 255;
			}
			else {
				pixelsThisRow_high[j] = 0;
				if (pixel <= lowThreshold) {
					pixelsThisRow_low[j] = 0;
				}
				else {
					pixelsThisRow_low[j] = 255;
				}
			}
		}
	}
}

void LinkEdge(Mat &imageOutput, const Mat &lowThresImage, const Mat &highThresImage) {
	imageOutput = highThresImage.clone();
	int rowCount = imageOutput.rows;
	int columnCount = imageOutput.cols;
	// 为计算方便，牺牲图像四周1像素宽的一圈
	for (int i = 1; i < rowCount - 1; i++) {
		uchar *pixelsPreviousRow = imageOutput.ptr<uchar>(i - 1);
		uchar *pixelsThisRow = imageOutput.ptr<uchar>(i);
		uchar *pixelsNextRow = imageOutput.ptr<uchar>(i + 1);
		for (int j = 1; j < columnCount - 1; j++) {
			if (pixelsThisRow[j] == 255) {
				GoAhead(i, j, pixelsPreviousRow, pixelsThisRow, pixelsNextRow, lowThresImage, imageOutput);
			}
			if (pixelsNextRow[j - 1] == 255) {
				GoAhead(i + 1, j - 1, pixelsThisRow, pixelsNextRow, imageOutput.ptr<uchar>(i + 1), lowThresImage,
					imageOutput);
			}
			if (pixelsNextRow[j] == 255) {
				GoAhead(i + 1, j, pixelsThisRow, pixelsNextRow, imageOutput.ptr<uchar>(i + 1), lowThresImage,
					imageOutput);
			}
			if (pixelsNextRow[j + 1] == 255) {
				GoAhead(i + 1, j + 1, pixelsThisRow, pixelsNextRow, imageOutput.ptr<uchar>(i + 1), lowThresImage,
					imageOutput);
			}
		}
	}
}

void GoAhead(int i, int j, uchar *pixelsPreviousRow, uchar *pixelsThisRow, uchar *pixelsNextRow, const Mat &lowThresImage, Mat &imageOutput) {
	// 判断左下方、右方、下方和右下方是否接续
	if (pixelsThisRow[j + 1] != 255 && pixelsNextRow[j + 1] != 255 && pixelsNextRow[j] != 255 &&
		pixelsNextRow[j - 1] != 255) {
		// 若不接续，从低阈值图中查找8领域是否接续，并对左上方、上方、右上方和左上方递归调用自身
		const uchar *pixelsPreviousRow_low = lowThresImage.ptr<uchar>(i - 1);
		const uchar *pixelsThisRow_low = lowThresImage.ptr<uchar>(i);
		const uchar *pixelsNextRow_low = lowThresImage.ptr<uchar>(i + 1);
		// 左上
		if (pixelsPreviousRow_low[j - 1] == 255) {
			pixelsPreviousRow[j - 1] = 255;
			if (i != 0 && j != 0) {
				GoAhead(i - 1, j - 1, imageOutput.ptr<uchar>(i - 1), pixelsPreviousRow, pixelsThisRow, lowThresImage,
					imageOutput);
			}
		}
		// 上
		if (pixelsPreviousRow_low[j] == 255) {
			pixelsPreviousRow[j] = 255;
			if (i != 0) {
				GoAhead(i - 1, j, imageOutput.ptr<uchar>(i - 1), pixelsPreviousRow, pixelsThisRow, lowThresImage,
					imageOutput);
			}
		}
		// 右上
		if (pixelsPreviousRow_low[j + 1] == 255) {
			pixelsPreviousRow[j + 1] = 255;
			if (i != 0 && j != imageOutput.cols) {
				GoAhead(i - 1, j + 1, imageOutput.ptr<uchar>(i - 1), pixelsPreviousRow, pixelsThisRow, lowThresImage,
					imageOutput);
			}
		}
		// 左
		if (pixelsThisRow_low[j - 1] == 255) {
			pixelsThisRow[j - 1] = 255;
			if (i != 0 && j != 0) {
				GoAhead(i - 1, j - 1, imageOutput.ptr<uchar>(i - 1), pixelsPreviousRow, pixelsThisRow, lowThresImage,
					imageOutput);
			}
		}
		// 右
		if (pixelsThisRow_low[j + 1] == 255) {
			pixelsThisRow[j + 1] = 255;
		}
		// 左下
		if (pixelsNextRow_low[j - 1] == 255) {
			pixelsNextRow[j - 1] = 255;
		}
		// 下
		if (pixelsNextRow_low[j] == 255) {
			pixelsNextRow[j] = 255;
		}
		// 右下
		if (pixelsNextRow_low[j + 1] == 255) {
			pixelsNextRow[j + 1] = 255;
		}
	}
}

void onParaChange(int, void *) {
	imageResult = Canny(imageOriginal, lowThreshold, highThreshold + 100);
	imshow(WINDOW_NAME, imageResult);
}