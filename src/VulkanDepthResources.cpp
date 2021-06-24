#include "VulkanDepthResources.h"

VkAttachmentDescription VulkanDepthResources::getDepthAttachmentDescription(VkPhysicalDevice physicalDevice) const
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat(physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depthAttachment;
}

VkAttachmentReference VulkanDepthResources::getDepthAttachmentReference() const
{
	VkAttachmentReference depthAttachmentReference{};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depthAttachmentReference;
}

void VulkanDepthResources::lazyInit(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VkCommandPool commandPool,
	VkQueue queue,
	uint32_t swapChainWidth,
	uint32_t swapChainHeight )
{
	mPhysicalDevice = physicalDevice;
	mLogicalDevice = logicalDevice;
	mCommandPool = commandPool;
	mQueue = queue;

	VkFormat depthFormat = findDepthFormat(mPhysicalDevice);

	createImage(
		swapChainWidth,
		swapChainHeight,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	createPersistentImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
}