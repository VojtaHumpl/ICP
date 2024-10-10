#pragma once

#include <thread>
#include <tuple>

// OpenCV 
#include <opencv2\opencv.hpp>

#include "ThreadSafeQueue.h"

class App {
public:
	App();
	~App();

	void init(void);
	int run(void);

private:
	void draw_cross(cv::Mat& img, int x, int y, int size);
	void draw_cross_normalized(cv::Mat& img, const cv::Point2f center_normalized, const int size);
	cv::Point2f find_object(const cv::Mat& img);
	cv::Mat threshold(const cv::Mat& img, const double h_low, const double s_low, const double v_low, const double h_hi, const double s_hi, const double v_hi);

	void camera_thread_function();
	void processing_thread_function();
	void gui_thread_function();

	cv::VideoCapture video_capture;
	ThreadSafeQueue<cv::Mat> frame_queue;
	ThreadSafeQueue<std::tuple<cv::Mat, std::string>> display_queue;

	std::thread gui_thread;
	std::thread camera_thread;
	std::thread processing_thread;
	bool stop_signal = false;
};
