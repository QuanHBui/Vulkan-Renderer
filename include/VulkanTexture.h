#pragma once

#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include <string>

#include "VulkanBuffer.h"

// Maybe one texture can hold multiple images?
class VulkanTexture : public VulkanBaseObject
{
public:
	VulkanTexture(const char *, VkDevice, VkPhysicalDevice, VkMemoryPropertyFlags);

private:
	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
};

#endif // VULKAN_TEXTURE_H