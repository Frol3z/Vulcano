#include "Window.hpp"

#include <iostream>

Window::Window() {
	if (!glfwInit()) {
		std::cerr << "GLFW initialization failed!";
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_pWindow = glfwCreateWindow(640, 480, "Vulcano", NULL, NULL);
 }

Window::~Window() {
	glfwTerminate();
}