#include "VulkanTexture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
}

VulkanTexture::VulkanTexture(const char *fileName, VkDevice device, VkPhysicalDevice physicalDevice)
{
	int texWidth, texHeight, texChannels;

	stbi_uc *pixels = vkTextureUtils::createTextureImage(fileName, &texWidth, &texHeight, &texChannels);

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	VulkanBuffer stagingBuffer
	{
		device,
		physicalDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	// Send data to staging buffer
	stagingBuffer.uploadData(pixels, imageSize);

	stbi_image_free(pixels);

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

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
}