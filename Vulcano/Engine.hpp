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
	bool m_bUseValidationLayers = true;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	// Swapchain related stuff (maybe later move it to a dedicated class)
	VkSwapchainKHR m_swapchain;
	VkFormat m_swapchainImageFormat;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkExtent2D m_swapchainExtent;
};