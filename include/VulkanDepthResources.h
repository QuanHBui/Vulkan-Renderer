#pragma once

#ifndef VULKAN_DEPTH_RESOURCES_H
#define VULKAN_DEPTH_RESOURCES_H

#include "VulkanUtils.h"
#include "VulkanImage.h"

class VulkanDepthResources : public VulkanImage
{
public:
	VulkanDepthResources() = default;

	VulkanDepthResources(VkPhysicalDevice, VkDevice);

	VkAttachmentDescription getDepthAttachmentDescription() const;
	VkAttachmentReference getDepthAttachmentReference() const;

private:
	VkFormat findDepthFormat() const
	{
		return vkutils::findSupportedFormat(
			mPhysicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
};

#endif // VULKAN_DEPTH_RESOURCES_H