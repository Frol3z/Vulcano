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

	// Grab the instance
	m_instance = returnedInstance.value().instance;
	m_debugMessenger = returnedInstance.value().debug_messenger;
}

void Engine::run() {
	while (!m_window.shouldClose()) {
		m_window.pollEvents();
	}
}

void Engine::clean() {
	vkb::destroy_debug_utils_messenger(m_instance, m_debugMessenger);
	vkDestroyInstance(m_instance, nullptr);
}