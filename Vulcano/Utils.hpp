#pragma once

#include "vulkan/vulkan.h"

namespace Utils {
	void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);

	VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd);
	VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
		VkSemaphoreSubmitInfo* waitSemaphoreInfo);
}