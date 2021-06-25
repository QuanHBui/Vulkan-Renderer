#include "VulkanTexture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanBuffer.h"
#include "VulkanCommandBuffers.h"
#include "VulkanImage.h"

namespace vkTextureUtils
{
	stbi_uc *loadTextureImage(std::string fileName, int *pTexWidth, int *pTexHeight, int *pTexChannels)
	{
		stbi_uc *pixels = stbi_load(fileName.c_str(), pTexWidth, pTexHeight, pTexChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		return pixels;
	}
}

VulkanTexture::VulkanTexture(
	std::string fileName,
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VkMemoryPropertyFlags properties,
	VkCommandPool commandPool,
	VkQueue queue )
	: VulkanImage(physicalDevice, logicalDevice, commandPool, queue)
	, mFileName(fileName)
{
	createTextureImage(properties);
	createTextureImageView();
	createTextureSampler();
}

void VulkanTexture::lazyInit(
	std::string fileName,
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VkMemoryPropertyFlags properties,
	VkCommandPool commandPool,
	VkQueue queue )
{
	mPhysicalDevice = physicalDevice;
	mLogicalDevice = logicalDevice;
	mCommandPool = commandPool;
	mQueue = queue;
	mFileName = fileName;

	createTextureImage(properties);
	createTextureImageView();
	createTextureSampler();
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
		mImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(mLogicalDevice, mCommandPool, mQueue, commandBuffer);
}

void VulkanTexture::createTextureImage(VkMemoryPropertyFlags properties)
{
	int texWidth, texHeight, texChannels;

	stbi_uc *pixels = vkTextureUtils::loadTextureImage(mFileName, &texWidth, &texHeight, &texChannels);

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

	createImage(
		texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		properties
	);

	// vkCmdCopyBufferToImage requires the image to be in the right layout first.
	transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer.getBufferHandle());

	stagingBuffer.cleanUp();
}

void VulkanTexture::createTextureImageView()
{
	createPersistentImageView(VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanTexture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	samplerInfo.magFilter = VK_FILTER_LINEAR; // For when oversampling
	samplerInfo.minFilter = VK_FILTER_LINEAR; // For when undersampling

	// What happen when going beyond the image dimension
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Enable/Disable anisotropic filtering. Performance hit.
	samplerInfo.anisotropyEnable = VK_TRUE;

	// Query and use the maximum amount of texels calculate the final color. Hardware dependent.
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// Specify which color to return when sampling beyond the image when in
	//  clamp to border mode
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	// Specify to use normalized u,v,w coordinates
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// Mainly for percentage-closer filtering
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	// Mipmapping
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(mLogicalDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler!");
	}
}