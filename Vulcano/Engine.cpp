#include "Engine.hpp"

#include <iostream>
#include "vendors/VkBootstrap.h"
#include "Utils.hpp"

#define VK_CHECK(x)														\
	do																	\
	{																	\
		VkResult err = x;												\
		if (err)														\
		{																\
			std::cout <<"Detected Vulkan error: " << err << std::endl;	\
			abort();													\
		}																\
	} while (0)

Engine::Engine() {
	initVulkan();
	initSwapchain();
	initCommands();
	initSyncStructures();
}

Engine::~Engine() {
	clean();
}

void Engine::run() {
	while (!m_window.shouldClose()) {
		m_window.pollEvents();
		draw();
	}
}

void Engine::initVulkan() {
	// Build the Vulkan instance
	vkb::InstanceBuilder instanceBuilder;
	auto returnedInstance = instanceBuilder.set_app_name("Vulcano")
		.request_validation_layers(m_bUseValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();
	if (!returnedInstance) {
		std::cerr << "Failed to create Vulkan instance. Error: " << returnedInstance.error().message() << "\n";
	}
	vkb::Instance vkbInstance = returnedInstance.value();
	m_instance = vkbInstance.instance;
	m_debugMessenger = vkbInstance.debug_messenger;

	// Create VK_KHR_SURFACE
	if (glfwCreateWindowSurface(m_instance, m_window.getRawPtr(), NULL, &m_surface)) {
		std::cerr << "Failed to create VK_KHR_SURFACE!" << std::endl;
	}

	// Vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features13.dynamicRendering = true;
	features13.synchronization2 = true;

	// Vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	// Choose a physical device (GPU)
	vkb::PhysicalDeviceSelector physicalDeviceSelector{ vkbInstance };
	vkb::PhysicalDevice vkbPhysicalDevice = physicalDeviceSelector
		.set_minimum_version(1, 3)
		.set_required_features_13(features13)
		.set_required_features_12(features12)
		.set_surface(m_surface)
		.select()
		.value();

	// Build logical device
	vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();
	m_physicalDevice = vkbPhysicalDevice.physical_device;
	m_device = vkbDevice.device;

	// Get a graphic queue
	m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void Engine::initSwapchain() {
	createSwapchain(m_window.WIDTH, m_window.HEIGHT);
}

void Engine::initCommands() {
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = m_graphicsQueueFamily;

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_frames[i].m_commandPool));

		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = m_frames[i].m_commandPool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i].m_mainCommandBuffer));
	}
}

void Engine::initSyncStructures() {
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_frames[i].m_renderFence));

		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frames[i].m_renderSemaphore));
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frames[i].m_swapchainSemaphore));
	}
}

void Engine::clean() {

	vkDeviceWaitIdle(m_device);

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		vkDestroyCommandPool(m_device, m_frames[i].m_commandPool, nullptr);

		// Destroy sync objects
		vkDestroyFence(m_device, m_frames[i].m_renderFence, nullptr);
		vkDestroySemaphore(m_device, m_frames[i].m_renderSemaphore, nullptr);
		vkDestroySemaphore(m_device, m_frames[i].m_swapchainSemaphore, nullptr);
	}

	destroySwapchain();
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debugMessenger);
	vkDestroyInstance(m_instance, nullptr);
}

void Engine::draw() {
	// Wait until GPU has finished rendering the last frame (timeout of 1s)
	VK_CHECK(vkWaitForFences(m_device, 1, &getCurrentFrame().m_renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(m_device, 1, &getCurrentFrame().m_renderFence));

	// Request image from the swapchain
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, getCurrentFrame().m_swapchainSemaphore, 
		nullptr, &swapchainImageIndex)
	);

	// Resetting command buffer
	VkCommandBuffer cmd = getCurrentFrame().m_mainCommandBuffer;
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	// Begin command buffer recording
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;
	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// Make the swapchain image writeable
	Utils::transitionImage(cmd, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// Color flash
	VkClearColorValue clearValue;
	float flash = std::abs(std::sin(m_frameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

	VkImageSubresourceRange clearRange = Utils::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//Cclear image
	vkCmdClearColorImage(cmd, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

	// Make the swapchain image to presentable
	Utils::transitionImage(cmd, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// End command buffer recording
	VK_CHECK(vkEndCommandBuffer(cmd));


	// Preparing for submission to the queue 
	VkCommandBufferSubmitInfo cmdinfo = Utils::commandBufferSubmitInfo(cmd);
	VkSemaphoreSubmitInfo waitInfo = Utils::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame().m_swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = Utils::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame().m_renderSemaphore);

	VkSubmitInfo2 submit = Utils::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

	// Submit command buffer to the queue and execute it
	VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submit, getCurrentFrame().m_renderFence));

	// Present image to the screen
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &getCurrentFrame().m_renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(m_graphicsQueue, &presentInfo));

	// Increase the number of frames drawn
	m_frameNumber++;
}

void Engine::createSwapchain(uint32_t width, uint32_t height) {
	// Build swapchain
	vkb::SwapchainBuilder swapchainBuilder(m_physicalDevice, m_device, m_surface);
	m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.set_desired_format(VkSurfaceFormatKHR{ m_swapchainImageFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(m_window.WIDTH, m_window.HEIGHT)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();
	m_swapchainExtent = vkbSwapchain.extent;
	m_swapchain = vkbSwapchain.swapchain;
	m_swapchainImages = vkbSwapchain.get_images().value();
	m_swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Engine::destroySwapchain() {
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	for (int i = 0; i < m_swapchainImageViews.size(); i++) {
		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}
}