#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
	const unsigned int WIDTH = 800;
	const unsigned int HEIGHT = 600;
public:
	Window();
	~Window();

	GLFWwindow* getRawPtr() const { return m_pWindow; }
	int shouldClose() const { return glfwWindowShouldClose(m_pWindow); }
	void pollEvents() const { glfwPollEvents(); }

private:
	GLFWwindow* m_pWindow;
};