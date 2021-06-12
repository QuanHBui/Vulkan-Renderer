#pragma once

#ifndef VULKAN_TEXTURE_H
#define VULKAN_TEXTURE_H

#include <string>

#include "VulkanBuffer.h"

// Maybe one texture can hold multiple images?
class VulkanTexture
{
public:
	VulkanTexture(const char *, VkDevice, VkPhysicalDevice);

private:
};

#endif // VULKAN_TEXTURE_H