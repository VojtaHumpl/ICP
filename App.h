#pragma once

// OpenCV 
#include <opencv2\opencv.hpp>

class App {
public:
	App();

	void init(void);
	int run(void);

	void draw_cross(cv::Mat& img, int x, int y, int size);
	void draw_cross_normalized(cv::Mat& img, const cv::Point2f center_normalized, const int size);
	cv::Point2f find_object(const cv::Mat& img);
	cv::Mat threshold(const cv::Mat& img, const double h_low, const double s_low, const double v_low, const double h_hi, const double s_hi, const double v_hi);

	~App();

private:

	cv::VideoCapture capture;  // global variable, move to app class, protected
};
