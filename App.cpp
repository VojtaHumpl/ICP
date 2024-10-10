// icp.cpp 
// Author: Vojtěch Humpl & David Jansa

// C++ 
#include <iostream>
#include <chrono>
#include <stack>
#include <random>
#include <numeric>

#include "App.h"

using namespace std;

App::App() {
	cout << "OpenCV: " << CV_VERSION << endl;
}

void App::init(void) {
	video_capture = cv::VideoCapture(cv::CAP_MSMF);

	//open video file
	//capture = cv::VideoCapture("video.mkv");

	if (!video_capture.isOpened()) {
		cerr << "no source?" << endl;
		exit(EXIT_FAILURE);
	} else {
		cout << "Source: " <<
			": width=" << video_capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
			", height=" << video_capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
	}
}

int App::run(void) {
	cv::Mat frame, scene;

	camera_thread = thread(&App::camera_thread_function, this);
	processing_thread = thread(&App::processing_thread_function, this);
	gui_thread = thread(&App::gui_thread_function, this);

	camera_thread.join();
	processing_thread.join();
	gui_thread.join();

	return EXIT_SUCCESS;
}

void App::camera_thread_function() {
	cv::Mat frame;

    while (!stop_signal) {
		auto start = chrono::high_resolution_clock::now();

        video_capture.read(frame);
        if (frame.empty()) {
            cerr << "Camera disconnected or end of stream.\n";
            stop_signal = true;
            break;
        }
        frame_queue.push(frame);

		display_queue.push(make_tuple(frame, "Original Frame"));

		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed_microseconds = end - start;
		auto ms = chrono::duration_cast<chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
		cout << "Elapsed time capturing: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;
    
		if (cv::pollKey() == 27) {
			stop_signal = true;
			break;
		}
	}
}

void App::processing_thread_function() {
	cv::Mat frame;

	while (!stop_signal) {
		if (frame_queue.pop(frame)) {
			if (frame.empty()) {
				continue;
			}

			auto start = chrono::high_resolution_clock::now();

			cv::Point2f center = find_object(frame);
			cv::Point2f center_normalized(center.x / frame.cols, center.y / frame.rows);

			cv::Mat scene_cross;
			frame.copyTo(scene_cross);
			draw_cross_normalized(scene_cross, center_normalized, 30);

			display_queue.push(make_tuple(scene_cross, "Processed Frame"));

			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed_microseconds = end - start;
			auto ms = chrono::duration_cast<chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
			cout << "Elapsed time processing: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;

			if (cv::pollKey() == 27) {
				stop_signal = true;
				break;
			}
		}
	}
}

void App::gui_thread_function() {
	cv::Mat frame;
	tuple<cv::Mat, string> display;

	while (!stop_signal) {
		if (display_queue.pop(display)) {
			frame = get<0>(display);
			string window_name = get<1>(display);
			if (frame.empty()) {
				continue;
			}

			cv::imshow(window_name, frame);

			if (cv::pollKey() == 27) {
				stop_signal = true;
				break;
			}
		}
	}
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
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
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
	center_normalized.x = clamp(center_normalized.x, 0.0f, 1.0f);
	center_normalized.y = clamp(center_normalized.y, 0.0f, 1.0f);
	size = clamp(size, 1, min(img.cols, img.rows));

	cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

	draw_cross(img, center_absolute.x, center_absolute.y, size);
}

App::~App() {
	stop_signal = true;
	frame_queue.clear();

	cv::destroyAllWindows();

	if (video_capture.isOpened())
		video_capture.release();

	cout << "Application exited cleanly.\n";
}


