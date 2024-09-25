#pragma once

#include "Window.hpp"

#include "vendors/VkBootstrap.h"

class Engine {
public:
	Engine();
	~Engine();

	void init();
	void run();
	void clean();
private:
	Window m_window;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	bool m_bUseValidationLayers = true;
};