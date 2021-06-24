#pragma once

#ifndef VULKAN_DEPTH_RESOURCES_H
#define VULKAN_DEPTH_RESOURCES_H

#include <cassert>

#include "VulkanUtils.h"
#include "VulkanImage.h"

class VulkanDepthResources : public VulkanImage
{
public:
	VulkanDepthResources() = default;

	// This is not ready. Idk in what situation to use this constructor.
	VulkanDepthResources(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue)
		: VulkanImage(physicalDevice, logicalDevice, commandPool, queue) {}

	// Copy assignment
	VulkanDepthResources &operator=(const VulkanDepthResources &) = delete;

	// Move assignment
	VulkanDepthResources &operator=(VulkanDepthResources &&) = delete;

	VkAttachmentDescription getDepthAttachmentDescription(VkPhysicalDevice) const;
	VkAttachmentReference getDepthAttachmentReference() const;

	void lazyInit(VkPhysicalDevice, VkDevice, VkCommandPool, VkQueue, uint32_t, uint32_t);

private:
	VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) const
	{
		assert(physicalDevice != VK_NULL_HANDLE && "Physical device handle is null!");

		return vkutils::findSupportedFormat(
			physicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
};

#endif // VULKAN_DEPTH_RESOURCES_H