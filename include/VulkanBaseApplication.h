#pragma once

#ifndef VULKAN_BASE_APPLICATION_H
#define VULKAN_BASE_APPLICATION_H

#include <vector>
#include <vulkan/vulkan.h>

extern const bool enableValidationLayers;
extern const std::vector<const char *> validationLayers;

class VulkanBaseApplication
{
private:
	VkInstance vulkanInstance;
	VkDebugUtilsMessengerEXT debugMessenger;

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &);
	bool checkValidationLayerSupport();

public:
	void createVulkanInstance
		(const VkApplicationInfo *, std::vector<const char *> const &);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT *,
		void *);

	void cleanUp();

	inline VkInstance getVulkanInstance() const { return vulkanInstance; }
};

#endif // VULKAN_BASE_APPLICATION_H