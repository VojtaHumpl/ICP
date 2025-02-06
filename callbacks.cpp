#include <iostream>

#include "App.h"


void App::GLFWErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << std::endl;
}

void App::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			// Exit The App
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_V:
			// Vsync on/off
			this_inst->isVsyncOn = !this_inst->isVsyncOn;
			glfwSwapInterval(this_inst->isVsyncOn);
			std::cout << "VSync: " << this_inst->isVsyncOn << "\n";
			break;
		case GLFW_KEY_I:
			this_inst->showImgui = !this_inst->showImgui;
			break;
		case GLFW_KEY_F:
			this_inst->fullscreen = !this_inst->fullscreen;
			if (this_inst->fullscreen) {
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				this_inst->prevWindowWidth = this_inst->windowWidth;
				this_inst->prevWindowHeight = this_inst->windowHeight;
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			} else {
				glfwSetWindowMonitor(window, nullptr, 0, 0, this_inst->prevWindowWidth, this_inst->prevWindowHeight, GLFW_DONT_CARE);
				glfwSetWindowPos(window, this_inst->windowX, this_inst->windowY);
			}
			break;
		case GLFW_KEY_G:
			this_inst->cameraDetached = !this_inst->cameraDetached;
			if (!this_inst->cameraDetached && this_inst->player) {
				this_inst->camera.position = this_inst->player->getHeadPosition();
			}
			break;
		case GLFW_KEY_U:
			if (this_inst->player) {
				gAudioPlayer.playSound3D("SNEEZE", this_inst->camera.position.x + 10, this_inst->camera.position.y, this_inst->camera.position.z, this_inst->camera.position.x, this_inst->camera.position.y, this_inst->camera.position.z, 0.0f, 0.0f, -1.0f);
			}
			break;
		case GLFW_KEY_P:
			gAudioPlayer.cleanFinishedSounds();
			break;
		default:
			break;
		}
	}
}

void App::GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (yoffset > 0) {
		this_inst->camera.ProcessMouseScroll(1.0f);
	} else if (yoffset < 0) {
		this_inst->camera.ProcessMouseScroll(-1.0f);
	}
}

void App::GLFWFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	this_inst->windowWidth = width;
	this_inst->windowHeight = height;
	glViewport(0, 0, width, height);
}

void App::GLFWWindowPosCallback(GLFWwindow* window, int xpos, int ypos) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (this_inst->fullscreen)
		return;

	this_inst->windowX = xpos;
	this_inst->windowY = ypos;
}

void App::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT: {
			int mode = glfwGetInputMode(window, GLFW_CURSOR);
			if (mode == GLFW_CURSOR_NORMAL) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			} else {
				std::cout << "Bang!\n";
			}
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		default:
			break;
		}
	}
}

void App::GLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	static bool firstMouse = true;
	static double lastX = 0.0;
	static double lastY = 0.0;
	
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;
		this_inst->camera.ProcessMouseMovement(xoffset, yoffset);
	} else {
		firstMouse = true;
	}
}

void GLAPIENTRY App::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	auto const src_str = [source]() {
		switch (source) {
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		default: return "Unknown";
		}
		}();

	auto const type_str = [type]() {
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: return "Unknown";
		}
		}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: return "Unknown";
		}
		}();

	std::cout << "[GL CALLBACK]: " <<
		"source = " << src_str <<
		", type = " << type_str <<
		", severity = " << severity_str <<
		", ID = '" << id << '\'' <<
		", message = '" << message << '\'' << std::endl;
}