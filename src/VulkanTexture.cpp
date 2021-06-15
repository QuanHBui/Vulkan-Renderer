#include "VulkanTexture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanCommandBuffers.h"

namespace vkTextureUtils
{
	stbi_uc *loadTextureImage(const char *fileName, int *pTexWidth, int *pTexHeight, int *pTexChannels)
	{
		stbi_uc *pixels = stbi_load(fileName, pTexWidth, pTexHeight, pTexChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		return pixels;
	}
}

VulkanTexture::VulkanTexture(
	const char *fileName,
	VkDevice logicalDevice,
	VkPhysicalDevice physicalDevice,
	VkMemoryPropertyFlags properties,
	VkCommandPool commandPool,
	VkQueue queue)
{
	assert(logicalDevice != VK_NULL_HANDLE);
	assert(physicalDevice != VK_NULL_HANDLE);
	assert(commandPool != VK_NULL_HANDLE);
	assert(queue != VK_NULL_HANDLE);

	mLogicalDevice = logicalDevice;
	mPhysicalDevice = physicalDevice;
	mCommandPool = commandPool;
	mQueue = queue;

	int texWidth, texHeight, texChannels;

	stbi_uc *pixels = vkTextureUtils::loadTextureImage(fileName, &texWidth, &texHeight, &texChannels);

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	mWidth = texWidth;
	mHeight = texHeight;

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

	transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer.getBufferHandle());
}

// vkCmdCopyBufferToImage requires the image to be in the right layout first. This function will change the layout of an
//  image when needed
void VulkanTexture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(mLogicalDevice, mCommandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	// Not using the barrier to transfer queue family ownership
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	// Image is not an array and does not have mipmapping levels => only 1 level and layer
	barrier.image = mTextureImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// Specify which types of operations involve the resource must happen before the barrier,
	//  and which operations that involve the resource must wait on the barrier
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::runtime_error("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(mLogicalDevice, mCommandPool, mQueue, commandBuffer);
}

void VulkanTexture::copyBufferToImage(VkBuffer buffer)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(mLogicalDevice, mCommandPool);

	// Specify which part of the buffer to be copied to which part of the image
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { mWidth, mHeight, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		mTextureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(mLogicalDevice, mCommandPool, mQueue, commandBuffer);
}