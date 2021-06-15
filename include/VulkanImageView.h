#pragma once

#ifndef VULKAN_IMAGE_VIEW_H
#define VULKAN_IMAGE_VIEW_H

#include <stdexcept>

#include <vulkan/vulkan.h>

VkImageView createImageView(VkDevice, VkImage, VkFormat);

#endif // VULKAN_IMAGE_VIEW_H