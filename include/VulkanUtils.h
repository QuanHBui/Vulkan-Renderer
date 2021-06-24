#pragma once

#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vector>

#include <vulkan/vulkan.h>

namespace vkutils
{
	VkResult createDebugUtilsMessengerEXT(
		VkInstance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger
	);

	void destroyDebugUtilsMessengerEXT(
		VkInstance,
		VkDebugUtilsMessengerEXT,
		const VkAllocationCallbacks *pAllocator
	);

	VkFormat findSupportedFormat(
		VkPhysicalDevice,
		const std::vector<VkFormat> &,
		VkImageTiling,
		VkFormatFeatureFlags
	);

	bool hasStencilComponent(VkFormat);
}

#endif // VULKAN_UTILS_H