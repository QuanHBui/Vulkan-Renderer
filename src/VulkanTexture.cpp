#include "VulkanTexture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanCommandBuffers.h"

namespace vkTextureUtils
{
	stbi_uc *createTextureImage(const char *fileName, int *pTexWidth, int *pTexHeight, int *pTexChannels)
	{
		stbi_uc *pixels = stbi_load(fileName, pTexWidth, pTexHeight, pTexChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		return pixels;
	}

	// vkCmdCopyBufferToImage requires the image to be in the right layout first. This function will change the layout of an
	//  image when needed
	void transitionImageLayout(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);

		endSingleTimeCommands(logicalDevice, commandPool, queue, commandBuffer);
	}
}

VulkanTexture::VulkanTexture(
	const char *fileName, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties)
{
	assert(logicalDevice != VK_NULL_HANDLE);
	assert(physicalDevice != VK_NULL_HANDLE);

	mLogicalDevice = logicalDevice;
	mPhysicalDevice = physicalDevice;

	int texWidth, texHeight, texChannels;

	stbi_uc *pixels = vkTextureUtils::createTextureImage(fileName, &texWidth, &texHeight, &texChannels);

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	VulkanBuffer stagingBuffer
	{
		mLogicalDevice,
		mPhysicalDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	// Send data to staging buffer
	stagingBuffer.uploadData(pixels, imageSize);

	stbi_image_free(pixels);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(texWidth);
	imageInfo.extent.height = static_cast<uint32_t>(texHeight);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (vkCreateImage(mLogicalDevice, &imageInfo, nullptr, &mTextureImage) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(mLogicalDevice, mTextureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &mTextureImageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory!");
	}

	vkBindImageMemory(mLogicalDevice, mTextureImage, mTextureImageMemory, 0);
}