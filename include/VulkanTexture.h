#pragma once

#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include <string>

#include "VulkanBuffer.h"

// Maybe one texture can hold multiple images?
class VulkanTexture : public VulkanBaseObject
{
public:
	VulkanTexture(const char *, VkDevice, VkPhysicalDevice, VkMemoryPropertyFlags, VkCommandPool, VkQueue);

	void cleanUp()
	{
		vkDestroyImage(mLogicalDevice, mTextureImage, nullptr);
		vkFreeMemory(mLogicalDevice, mTextureImageMemory, nullptr);
	}

private:
	void transitionImageLayout(VkFormat, VkImageLayout, VkImageLayout);
	void copyBufferToImage(VkBuffer);

	VkImage mTextureImage = VK_NULL_HANDLE;
	VkDeviceMemory mTextureImageMemory = VK_NULL_HANDLE;
	uint32_t mWidth = 0, mHeight = 0;

	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	VkQueue mQueue = VK_NULL_HANDLE;
};

#endif // VULKAN_TEXTURE_H