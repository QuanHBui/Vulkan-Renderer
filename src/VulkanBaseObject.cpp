#include "VulkanBaseObject.h"

#include <stdexcept>

void VulkanBaseObject::allocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties)
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		mPhysicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &mMemoryHandle) != VK_SUCCESS) {
		throw std::runtime_error("[ERROR] Failed to allocate memory for VulkanBaseObject!");
	}
}