// icp.cpp 
// Author: Vojtěch Humpl & David Jansa

// C++ 
#include <iostream>
#include <chrono>
#include <stack>
#include <random>
#include <numeric>

#include "App.h"


App::App() {
	std::cout << "OpenCV: " << CV_VERSION << std::endl;
}

void App::init(void) {
	//open first available camera
	capture = cv::VideoCapture(cv::CAP_DSHOW);

	//open video file
	//capture = cv::VideoCapture("video.mkv");

	if (!capture.isOpened()) {
		std::cerr << "no source?" << std::endl;
		exit(EXIT_FAILURE);
	} else {
		std::cout << "Source: " <<
			": width=" << capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
			", height=" << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
	}
}

int App::run(void) {
	cv::Mat frame, scene;

	while (1) {
		auto start = std::chrono::high_resolution_clock::now();

		capture.read(frame);
		if (frame.empty()) {
			std::cerr << "Cam disconnected? End of file?\n";
			break;
		}

		// show grabbed frame
		cv::imshow("grabbed", frame);

		// analyze the image...
		cv::Point2f center = find_object(frame);
		//cv::Mat img_threshold = threshold(frame, 128.0, 128.0, 128.0, 255.0, 255.0, 255.0);

		cv::Point2f center_normalized(center.x / frame.cols, center.y / frame.rows);

		// make a copy and draw center
		cv::Mat scene_cross;
		frame.copyTo(scene_cross);
		draw_cross_normalized(scene_cross, center_normalized, 30);
		cv::imshow("scene", scene_cross);

		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed_milliseconds = end - start;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_milliseconds).count();
		std::cout << "Elapsed time: " << ms << "ms, " << "FPS: " << 1000.0/ms <<  std::endl;


		if (cv::waitKey(1) == 27)
			break;
	}

	return EXIT_SUCCESS;
}

cv::Mat App::threshold(const cv::Mat& img, const double h_low, const double s_low, const double v_low, const double h_hi, const double s_hi, const double v_hi) {
	cv::Mat img_hsv, img_threshold;

	cv::cvtColor(img, img_hsv, cv::COLOR_BGR2HSV);

	cv::Scalar lower_threshold = cv::Scalar(h_low, s_low, v_low);
	cv::Scalar upper_threshold = cv::Scalar(h_hi, s_hi, v_hi);
	cv::inRange(img_hsv, lower_threshold, upper_threshold, img_threshold);

	return img_threshold;
}

cv::Point2f App::find_object(const cv::Mat& img) {
    // First range for red hue
    cv::Mat img_threshold1 = threshold(img, 0, 100, 100, 10, 255, 255);

    // Second range for red hue
    cv::Mat img_threshold2 = threshold(img, 170, 100, 100, 180, 255, 255);

    // Combine both ranges
    cv::Mat img_threshold;
    cv::bitwise_or(img_threshold1, img_threshold2, img_threshold);

	// find contours
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(img_threshold, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	// find the largest contour
	int largest_area = 0;
	int largest_contour_index = 0;
	for (size_t i = 0; i < contours.size(); i++) {
		double a = cv::contourArea(contours[i], false);
		if (a > largest_area) {
			largest_area = a;
			largest_contour_index = i;
		}
	}

	if (contours.empty())
		return cv::Point2f(0, 0);

	// find the center of the largest contour
	cv::Moments mu = cv::moments(contours[largest_contour_index]);
	cv::Point2f center(mu.m10 / mu.m00, mu.m01 / mu.m00);

	// if center nan, return (0,0)
	if (center.x != center.x || center.y != center.y)
		return cv::Point2f(0, 0);

	return center;
}

void App::draw_cross(cv::Mat& img, int x, int y, int size) {
	cv::Point p1(x - size / 2, y);
	cv::Point p2(x + size / 2, y);
	cv::Point p3(x, y - size / 2);
	cv::Point p4(x, y + size / 2);

	cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
	cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

void App::draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size) {
	center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
	center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
	size = std::clamp(size, 1, std::min(img.cols, img.rows));

	cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

	draw_cross(img, center_absolute.x, center_absolute.y, size);
}

App::~App() {
	// clean-up
	cv::destroyAllWindows();
	std::cout << "Bye...\n";

	if (capture.isOpened())
		capture.release();
}


