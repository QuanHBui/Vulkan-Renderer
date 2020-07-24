#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include <vulkan/vulkan.h>

/**
 * A generic vulkan buffer, it can be any buffer depending on the passed
 *  in VkBufferUsageFlags
 *
 * TODO: Main is handling the cleanup at the moment. This is for later design to see if this
 *  wrapper class should handle its own cleaup. It should. But, it needs to know information
 *  about the VkDevice from main. It might be messy to have every wrapper class to store info
 *  about VkDevice. Further design investigation is needed before making any decision.
 */
class VulkanBuffer
{
private:
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

	uint32_t findMemoryType(VkPhysicalDevice const &, uint32_t, VkMemoryPropertyFlags);

public:
	VulkanBuffer(
		VkDevice const &,
		VkPhysicalDevice const &,
		VkDeviceSize,
		VkBufferUsageFlags,
		VkMemoryPropertyFlags);

	inline const VkBuffer getBuffer() const { return buffer; }
	inline const VkDeviceMemory getBufferMemory() const { return bufferMemory; }
};

#endif // VULKAN_BUFFER_H