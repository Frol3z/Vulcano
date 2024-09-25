#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
	Window();
	~Window();

	int shouldClose() const { return glfwWindowShouldClose(m_pWindow); }
	void pollEvents() const { glfwPollEvents(); }
private:
	GLFWwindow* m_pWindow;
};