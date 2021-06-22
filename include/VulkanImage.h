#pragma once

#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "VulkanBaseObject.h"

VkImageView createImageView(VkDevice, VkImage, VkFormat, VkImageAspectFlags);

class VulkanImage : public VulkanBaseObject
{
public:
	VulkanImage() = default;
	VulkanImage(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue)
		: VulkanBaseObject(physicalDevice, logicalDevice), mCommandPool(commandPool), mQueue(queue) {};

	void createImage(VkImageCreateInfo imageInfo)
	{
		if (vkCreateImage(mLogicalDevice, &imageInfo, nullptr, &mImage) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!");
		}
	}

	void transitionImageLayout(VkFormat, VkImageLayout, VkImageLayout);

protected:
	VkImage mImage = VK_NULL_HANDLE;
	VkImageView mImageView = VK_NULL_HANDLE;

	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	VkQueue mQueue = VK_NULL_HANDLE;
};

#endif // VULKAN_IMAGE_VIEW_H