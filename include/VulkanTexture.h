#pragma once

#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include <string>

#include "VulkanBaseObject.h"
#include "VulkanImage.h"

// Maybe one texture can hold multiple images?
class VulkanTexture : public VulkanImage
{
public:
	VulkanTexture() = default;
	VulkanTexture(std::string, VkPhysicalDevice, VkDevice, VkMemoryPropertyFlags, VkCommandPool, VkQueue);

	void lazyInit(std::string, VkPhysicalDevice, VkDevice, VkMemoryPropertyFlags, VkCommandPool, VkQueue);

	VkImageView getTextureImageView() const { return mImageView; }
	VkSampler getTextureSampler() const { return mTextureSampler; }

	void cleanUp()
	{
		vkDestroySampler(mLogicalDevice, mTextureSampler, nullptr);
		vkDestroyImageView(mLogicalDevice, mImageView, nullptr);

		vkDestroyImage(mLogicalDevice, mImage, nullptr);
		vkFreeMemory(mLogicalDevice, mMemoryHandle, nullptr);
	}

private:
	void copyBufferToImage(VkBuffer);
	void createTextureImage(VkMemoryPropertyFlags);
	void createTextureImageView();
	void createTextureSampler();
	void generateMipmaps();

	uint32_t mWidth = 0, mHeight = 0, mMipLevels = 0;

	VkSampler mTextureSampler = VK_NULL_HANDLE;

	std::string mFileName;
};

#endif // VULKAN_TEXTURE_H