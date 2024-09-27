#include "Engine.hpp"

#include <iostream>
#include "vendors/VkBootstrap.h"

Engine::Engine() {
	init();
}

Engine::~Engine() {
	clean();
}

void Engine::init() {
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

void Engine::run() {
	while (!m_window.shouldClose()) {
		m_window.pollEvents();
	}
}

void Engine::clean() {

	// Destroy swapchain
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	for (int i = 0; i < m_swapchainImageViews.size(); i++) {
		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debugMessenger);
	vkDestroyInstance(m_instance, nullptr);
}