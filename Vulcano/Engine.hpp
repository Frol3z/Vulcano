#pragma once

#include "Window.hpp"

#include "vendors/VkBootstrap.h"

struct FrameData {
	VkCommandPool m_commandPool;
	VkCommandBuffer m_mainCommandBuffer;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class Engine {
public:
	Engine();
	~Engine();

	void run();

private:
	void initVulkan();
	void initSwapchain();
	void initCommands();
	void clean();

	void createSwapchain(uint32_t width, uint32_t height);
	void destroySwapchain();

	FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; };
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

	// Queue and command buffer's stuff
	int m_frameNumber{ 0 };
	FrameData m_frames[FRAME_OVERLAP];
	VkQueue m_graphicsQueue;
	uint32_t m_graphicsQueueFamily;
};