#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include <vulkan/vulkan.h>

/**
 * A generic vulkan buffer, it can be any buffer depending on the passed
 *  in VkBufferUsageFlags
 */
class VulkanBuffer
{
private:
	VkBuffer mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mBufferMemory = VK_NULL_HANDLE;

	VkDevice mLogicalDevice = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

	uint32_t findMemoryType(VkPhysicalDevice const &, uint32_t, VkMemoryPropertyFlags);

public:
	VulkanBuffer() = default;
	VulkanBuffer(
		VkDevice,
		VkPhysicalDevice,
		VkDeviceSize,
		VkBufferUsageFlags,
		VkMemoryPropertyFlags);

	void cleanUpBuffer();

	inline const VkBuffer getBuffer() const { return mBuffer; }
	inline const VkDeviceMemory getBufferMemory() const { return mBufferMemory; }
};

#endif // VULKAN_BUFFER_H