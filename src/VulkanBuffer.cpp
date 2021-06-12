#include "VulkanBuffer.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

/**
 * Graphics cards can offer different types of memory to allocate from, we need to find the right
 *  type of memory to use to allocate our buffer.
 *
 * @param: typeFilter - specify the bit field of memory types that are suitable
 */
uint32_t VulkanBuffer::findMemoryType(
	VkPhysicalDevice const &physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	// Query info about the available types of memory
	VkPhysicalDeviceMemoryProperties memProperties;	// We get memory types and memory heaps from this
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		/* The parameter typeFilter that passed in is from memoryTypeBits of struct VkMemoryRequirements.
			It is a bit field that sets a bit for every memoryType that is supported for the resource.
			So, we need to first check a bit at a certain index from VkPhysicalDeviceMemory is on by left shifting
			the number 1 by the amount of the index.

			After which, we need to check the propertyFlags of the memoryType at a certain index as well with
			similar AND bitwise operation fashion. However, we may have more than one desire property so we
			need to check not only if the result of the bitwise operation is non-zero, we also need to check
			if the bitwise operation result is equal to the desired bit properties bit filed.
		*/
		if (typeFilter & (1 << i) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("[ERROR] Failed to find suitable memory type!");
}

VulkanBuffer::VulkanBuffer(
	VkDevice logicalDevice,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties) :
	mLogicalDevice(logicalDevice), mPhysicalDevice(physicalDevice)
{
	// A logical device and physical device must be created prior to create a buffer
	assert(mLogicalDevice != VK_NULL_HANDLE);
	assert(mPhysicalDevice != VK_NULL_HANDLE);

	//============================ Create a buffer object ============================
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS) {
		throw std::runtime_error("[ERROR] Failed to create a buffer!");
	}

	// After specifying what our buffer going to be, we need to query what requirements we need to satisfy to create it
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, mBuffer, &memRequirements);

	//========================== Allocate memory on the GPU ==========================
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		physicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("[ERROR] Failed to allocate buffer memory!");
	}

	// Associate the memory with the buffer
	vkBindBufferMemory(logicalDevice, mBuffer, mBufferMemory, 0);
}

void VulkanBuffer::uploadData(void *data, VkDeviceSize size)
{
	void *pMappedMemory = nullptr;

	vkMapMemory(mLogicalDevice, mBufferMemory, 0, size, 0, &pMappedMemory);
	memcpy(pMappedMemory, data, static_cast<size_t>(size));
	vkUnmapMemory(mLogicalDevice, mBufferMemory);
}

void VulkanBuffer::cleanUpBuffer()
{
	vkDestroyBuffer(mLogicalDevice, mBuffer, nullptr);
	vkFreeMemory(mLogicalDevice, mBufferMemory, nullptr);
}