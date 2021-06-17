#pragma once

#include <QWidget>
#include "ui_PictureMg.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <ctime>

class PictureMg : public QWidget
{
	Q_OBJECT

public:
	PictureMg(QWidget *parent = Q_NULLPTR);
	~PictureMg();

private:
	Ui::PictureMg ui;

	cv::Mat image;		//	 ‰»ÎÕº
	cv::Mat gray;		//	 ‰»ÎÕºª“∂»Õº
	cv::Mat imageClone;	//	 ‰»ÎÕºøÀ¬°Õº
	cv::Mat result;		//	‘§¿¿Õº

	std::string nameWindow = "‘§¿¿Õº";
	QString path;		//	 ‰»ÎÕº¬∑æ∂

	int liangduValue = 0;
	int duibiduValue = 100;

	void loadImage();	//	º”‘ÿÕº∆¨
	void showMessage();	//	œ‘ æÕºœÒ–≈œ¢

private slots:
	void open();
	void save();
	void reset();
	void fanse();
	void fudiao();
	void manhua();
	void sumiao();
	void jingxiang();
	void fugu();
	void miaobian();
	void heibai();
	void suoxiao();
	void fangda();
	void shuzi();
	void bolang();
	void back();
};
