#pragma once

#ifndef VULKAN_BASE_OBJECT_H
#define VULKAN_BASE_OBJECT_H

#include <vulkan/vulkan.h>

class VulkanBaseObject
{
public:
	VulkanBaseObject() = default;
	VulkanBaseObject(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
		: mPhysicalDevice(physicalDevice), mLogicalDevice(logicalDevice) {};

	VkDeviceMemory getMemoryHandle() const { return mMemoryHandle; }

protected:
	void allocateMemory(VkMemoryRequirements, VkMemoryPropertyFlags);
	uint32_t findMemoryType(VkPhysicalDevice const &, uint32_t, VkMemoryPropertyFlags);

	VkDevice mLogicalDevice = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

	VkDeviceMemory mMemoryHandle = VK_NULL_HANDLE;
};

#endif // VULKAN_BASE_OBJECT_H