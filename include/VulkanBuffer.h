#pragma once

#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include "VulkanBaseObject.h"

/**
 * A generic vulkan buffer, it can be any buffer depending on the passed
 *  in VkBufferUsageFlags
 */
class VulkanBuffer : public VulkanBaseObject
{
public:
	// Cheap constructor
	VulkanBuffer() = default;

	// Costly constructor. Also allocate buffer without uploading data
	VulkanBuffer(
		VkDevice,
		VkPhysicalDevice,
		VkDeviceSize,
		VkBufferUsageFlags,
		VkMemoryPropertyFlags
	);

	VulkanBuffer(VulkanBuffer const &vulkanBuffer) = delete;

	// Only call this if you know what you are doing.
	// Parameters should be similar to the costly constructor.
	void lazyInit(
		VkDevice,
		VkPhysicalDevice,
		VkDeviceSize,
		VkBufferUsageFlags,
		VkMemoryPropertyFlags
	);

	void uploadData(void *, VkDeviceSize);
	void cleanUp();

	VkBuffer getBufferHandle() const { return mBuffer; }

private:
	void createBuffer();

	VkBuffer mBuffer = VK_NULL_HANDLE;
	VkDeviceSize mSize = 0;
	VkBufferUsageFlags mUsage;
	VkMemoryPropertyFlags mProperties;
};

#endif // VULKAN_BUFFER_H