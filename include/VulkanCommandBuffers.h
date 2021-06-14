#pragma once

#ifndef VULKAN_COMMAND_BUFFERS_H
#define VULKAN_COMMAND_BUFFERS_H

#include <vulkan/vulkan.h>

// Remember this commmand buffer is short-lived
VkCommandBuffer beginSingleTimeCommands(VkDevice, VkCommandPool);
void endSingleTimeCommands(VkDevice, VkCommandPool, VkQueue, VkCommandBuffer);

#endif // VULKAN_COMMAND_BUFFERS_H