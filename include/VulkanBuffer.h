#pragma once

#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include <vulkan/vulkan.h>

class VulkanBaseObject
{
public:

protected:
	uint32_t findMemoryType(VkPhysicalDevice const &, uint32_t, VkMemoryPropertyFlags);

	VkDevice mLogicalDevice = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
};

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
	void cleanUpBuffer();

	VkBuffer getBuffer() const { return mBuffer; }
	VkDeviceMemory getBufferMemory() const { return mBufferMemory; }

private:
	VkBuffer mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mBufferMemory = VK_NULL_HANDLE;
};

#endif // VULKAN_BUFFER_H