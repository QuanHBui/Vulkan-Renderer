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

	VkImageView getImageView() const { return mImageView; }

protected:
	void createImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags);

	void createPersistentImageView(VkImageAspectFlags aspectFlags)
	{
		mImageView = createImageView(mLogicalDevice, mImage, mFormat, aspectFlags);
	}

	void transitionImageLayout(VkImageLayout, VkImageLayout);

	VkImage mImage = VK_NULL_HANDLE;
	VkImageView mImageView = VK_NULL_HANDLE;
	VkFormat mFormat = VK_FORMAT_UNDEFINED;

	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	VkQueue mQueue = VK_NULL_HANDLE;
};

#endif // VULKAN_IMAGE_VIEW_H