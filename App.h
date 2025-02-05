#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>
#include <numeric>
#include <future>
#include <filesystem>
#include <atomic>

// OpenCV 
#include <opencv2\opencv.hpp>

#include <GL/glew.h> 
#include <GL/wglew.h> 

// GLFW toolkit
#include <GLFW/glfw3.h>

// OpenGL math
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "ThreadSafeQueue.h"
#include "ThreadPool.h"
#include "Assets.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "Camera.h"
#include "Player.h"
#include "CollisionManager.h"
#include "Entity.h"
#include "TerrainEntity.h"
#include "PhysicsEntity.h"


class App {
public:
	App();
	~App();

	void init();
	void initImgui();
	int run();
	void destroy();

private:
	// OpenGL
	GLFWwindow* window = nullptr;
	int windowX = 0;
	int windowY = 0;
	int windowWidth = 1280;
	int windowHeight = 720;
	int prevWindowWidth = windowWidth;
	int prevWindowHeight = windowHeight;
	bool isVsyncOn = true;
	bool fullscreen = false;

	std::atomic<bool> redDetected{ false };

	bool showImgui = true;
	float deltaTime = 0.0f;

	Player* player = nullptr;

	Camera camera;
	bool cameraDetached = false;

	DirectionalLight sunLight;
	//std::vector<Model> models;
	std::vector<ShaderProgram> shaders;
	std::vector<Entity*> entities;
	std::vector<PhysicsEntity*> physicsEntities;

	cv::VideoCapture videoCapture;
	ThreadSafeQueue<cv::Mat> frameQueue;
	ThreadSafeQueue<std::tuple<cv::Mat, std::string>> displayQueue;
	ThreadSafeQueue<cv::Mat> encodeQueue;
	ThreadSafeQueue<std::vector<uchar>> decodeQueue;

	// Threading
	ThreadPool threadPool;
	bool stopSignal = false;

	// Init
	void initOpenCV();
	void initGLEW();
	void initGLFW();
	void initGLDebug();
	void initAssets();

	void printInfoOpenCV();
	void printInfoGLFW();
	void printInfoGLM();
	void printInfoGL();

	void processInput(float deltaTime);

	void drawCross(cv::Mat& img, int x, int y, int size);
	void drawCrossNormalized(cv::Mat& img, const cv::Point2f center_normalized, const int size);
	cv::Point2f findObject(const cv::Mat& img);
	cv::Mat threshold(const cv::Mat& img, const double h_low, const double s_low, const double v_low, const double h_hi, const double s_hi, const double v_hi);
	std::vector<uchar> lossyLimitBW(cv::Mat& input_img, size_t size_limit);
	std::vector<uchar> lossyLimitQuality(cv::Mat& input_img, float target_quality);

	void cameraThreadFunction();
	void processingThreadFunction();
	void GUIThreadFunction();
	void encodeThreadFunction();
	void decodeThreadFunction();

	bool findRed(const cv::Mat& img);
	void cameraRedThreadFunction();
	void processingRedThreadFunction();

	
	//callbacks
	static void GLFWErrorCallback(int error, const char* description);
	static void GLFWFramebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void GLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void GLFWWindowPosCallback(GLFWwindow* window, int xpos, int ypos);
	static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
};
