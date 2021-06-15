#include "VulkanImageView.h"

VkImageView createImageView(VkDevice logicalDevice, VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;

	// Specify how image data should be interpreted
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Treat image as 2D texture
	viewInfo.format = format;

	// Describe what the image's purpose is and which part of the image should be accessed.
	//  In this case, images are used as color targets without mipmapping levels or multiple layers.
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture image view!");
	}

	return imageView;
}