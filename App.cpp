// Author: Vojtěch Humpl & David Jansa

// WARNING:
////// In general, you can NOT freely reorder includes!
//

// C++
// include anywhere, in any order
#include <iostream>
#include <chrono>
#include <stack>
#include <random>

// OpenCV (does not depend on GL)
#include <opencv2\opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h> 
// WGLEW = Windows GL Extension Wrangler (change for different platform) 
// platform specific functions (in this case Windows)
#include <GL/wglew.h> 

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
////// In general, you can NOT freely reorder includes!
#include "App.h"

using namespace std;

#include <thread>

App::App() : thread_pool(std::thread::hardware_concurrency()) {
	cout << "OpenCV: " << CV_VERSION << endl;
}

void App::init(void) {
	try {
		//open video file
		//video_capture = cv::VideoCapture("resources/video.mkv");

		video_capture = cv::VideoCapture(0, cv::CAP_MSMF);
		if (!video_capture.isOpened()) {
			//throw runtime_error("Can not open camera");
		} else {
			cout << "Source: " <<
				": width=" << video_capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
				", height=" << video_capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
		}

		thread_pool.enqueue(&App::camera_thread_function, this);
		thread_pool.enqueue(&App::processing_thread_function, this);
		thread_pool.enqueue(&App::gui_thread_function, this);
		thread_pool.enqueue(&App::encode_threaded_function, this);
	} catch (const exception& e) {
		cerr << "Init failed: " << e.what() << endl;
		throw;
	}

	cout << "Application initialized.\n";
}

int App::run(void) {
	cv::Mat frame, scene;

	//camera_thread = thread(&App::camera_thread_function, this);
	//processing_thread = thread(&App::processing_thread_function, this);
	//gui_thread = thread(&App::gui_thread_function, this);
	//
	//camera_thread.join();
	//processing_thread.join();
	//gui_thread.join();

	while (!stop_signal) {
		this_thread::sleep_for(chrono::milliseconds(100));
	}

	return EXIT_SUCCESS;
}

void App::camera_thread_function() {
	cv::Mat frame;

    while (!stop_signal) {
		auto start = chrono::high_resolution_clock::now();

        video_capture.read(frame);
        if (frame.empty() && display_queue.empty()) {
            cerr << "Camera disconnected or end of stream.\n";
            stop_signal = true;
            break;
        }
        frame_queue.push(frame);

		display_queue.push(make_tuple(frame, "Original Frame"));
		encode_queue.push(frame);

		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed_microseconds = end - start;
		auto ms = chrono::duration_cast<chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
		cout << "Elapsed time capturing: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;
	}
}

void App::encode_threaded_function() {
	cv::Mat frame;

	float target_coefficient = 0.5f;
	while (!stop_signal) {
		if (encode_queue.pop(frame)) {
			if (frame.empty()) {
				continue;
			}

			auto start = chrono::high_resolution_clock::now();

			auto size_uncompressed = frame.elemSize() * frame.total();
			auto size_compressed_limit = size_uncompressed * target_coefficient;
			vector<uchar> bytes = lossy_bw_limit(frame, size_compressed_limit);
			//vector<uchar> bytes = lossy_quality_limit(frame, 30.0f);

			decode_queue.push(bytes);

			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed_microseconds = end - start;
			auto ms = chrono::duration_cast<chrono::microseconds>(elapsed_microseconds).count() / 1000.0;
			cout << "Elapsed time encoding: " << ms << "ms, " << "FPS: " << 1000.0 / ms << endl;
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
		}
	}
}

void App::gui_thread_function() {
	cv::Mat frame;
	tuple<cv::Mat, string> display;
	//int target_fps = 15;

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

			//int ms_per_frame = 1000.0 / target_fps;
			//int key = cv::waitKey(ms_per_frame);
		}
	}
}

vector<uchar> App::lossy_bw_limit(cv::Mat& input_img, size_t size_limit) {
	std::string suff(".jpg"); // target format
	if (!cv::haveImageWriter(suff))
		throw std::runtime_error("Can not compress to format:" + suff);

	std::vector<uchar> bytes;
	std::vector<int> compression_params;

	// prepare parameters for JPEG compressor
	// we use only quality, but other parameters are available (progressive, optimization...)
	std::vector<int> compression_params_template;
	compression_params_template.push_back(cv::IMWRITE_JPEG_QUALITY);

	std::cout << '[';

	//try step-by-step to decrease quality by 5%, until it fits into limit
	for (auto i = 100; i > 0; i -= 5) {
		compression_params = compression_params_template; // reset parameters
		compression_params.push_back(i);                  // set desired quality
		std::cout << i << ',';

		// try to encode
		cv::imencode(suff, input_img, bytes, compression_params);

		// check the size limit
		if (bytes.size() <= size_limit)
			break; // ok, done 
	}
	std::cout << "]\n";

	return bytes;
}

vector<uchar> App::lossy_quality_limit(cv::Mat& input_img, float target_quality) {
	std::string suff(".jpg"); // Target format
    if (!cv::haveImageWriter(suff))
        throw std::runtime_error("Cannot compress to format: " + suff);

	std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, 95 };

    std::vector<uchar> best_bytes;
    int best_quality = -1;

	std::vector<int> qualities(100);
	std::iota(qualities.begin(), qualities.end(), 1);

	std::vector<std::future<void>> futures;
	std::mutex mtx;

	auto compress_and_evaluate_task = [&](int quality) {
		std::vector<uchar> bytes;
		std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, quality };
		cv::imencode(suff, input_img, bytes, params);

		cv::Mat decoded_img = cv::imdecode(bytes, cv::IMREAD_COLOR);

		// Compute PSNR between the original and compressed images
		double psnr = cv::PSNR(input_img, decoded_img);

		std::cout << "Quality: " << quality << ", PSNR: " << psnr << std::endl;

		// Update best quality if criteria met
		if (psnr >= target_quality) {
			std::lock_guard<std::mutex> lock(mtx);
			if (best_quality == -1 || quality < best_quality) {
				best_quality = quality;
				best_bytes = std::move(bytes); // Save the best compressed image
			}
		}
	};

	for (int quality : qualities) {
		futures.emplace_back(thread_pool.enqueue(compress_and_evaluate_task, quality));
	}

	for (auto& future : futures) {
		future.get();
	}

	if (best_quality == -1) {
		throw std::runtime_error("Cannot achieve target PSNR with available quality settings.");
	}

	std::cout << "Selected Quality: " << best_quality << std::endl;
	return best_bytes;
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
	display_queue.clear();
	encode_queue.clear();
	decode_queue.clear();

	cv::destroyAllWindows();

	if (video_capture.isOpened())
		video_capture.release();

	cout << "Application exited cleanly.\n";
}


