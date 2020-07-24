#pragma once

#ifndef	VULKAN_BASE_APPLICATION_H
#define VULKAN_BASE_APPLICATION_H

#include <vector>
#include <vulkan/vulkan.h>

class VulkanBaseApplication
{
private:
	VkInstance vulkanInstance;

public:
	void createVulkanInstance
		(const VkApplicationInfo *, std::vector<const char *> const &);
};

#endif // VULKAN_BASE_APPLICATION_H