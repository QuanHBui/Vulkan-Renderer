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
	VulkanBuffer() = default;

	// Alocate buffer without uploading data
	explicit VulkanBuffer(
		VkDevice,
		VkPhysicalDevice,
		VkDeviceSize,
		VkBufferUsageFlags,
		VkMemoryPropertyFlags
	);

	VulkanBuffer(VulkanBuffer const &vulkanBuffer) = delete;

	void uploadData(void *, VkDeviceSize);
	void cleanUp();

	VkBuffer getBufferHandle() const { return mBuffer; }

private:
	VkBuffer mBuffer = VK_NULL_HANDLE;
};

#endif // VULKAN_BUFFER_H