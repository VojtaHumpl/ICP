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

}

void App::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT: {
			int mode = glfwGetInputMode(window, GLFW_CURSOR);
			if (mode == GLFW_CURSOR_NORMAL) {
				// we are aoutside of applicating, catch the cursor
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			} else {
				// we are inside our game: shoot, click, etc.
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