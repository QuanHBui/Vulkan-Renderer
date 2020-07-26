#include "VulkanBaseApplication.h"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "VulkanUtils.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// List of validation layers to enable
const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

void VulkanBaseApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =	VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =	VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

/**
 * Check if all of the layers in validationLayers exists in the availableLayers list
 */
bool VulkanBaseApplication::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : validationLayers) {
		bool layerFound = false;

		for (const VkLayerProperties &layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

void VulkanBaseApplication::createVulkanInstance(
	const VkApplicationInfo *pAppInfo, std::vector<const char *> const &extensions)
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("[ERROR] Validation layers requested, but not available!");
	}

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = pAppInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// Enable validation layers if debug mode
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;

	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
		throw std::runtime_error("[ERROR] Failed to create a Vulkan instance");
	}

	// Set up debug messenger
	if (enableValidationLayers) {
		populateDebugMessengerCreateInfo(debugCreateInfo);

		if (vkutils::createDebugUtilsMessengerEXT(vulkanInstance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to set up debug messenger!");
		}
	}
}

/**
 * Return a boolean that indicates if the Vulkan call that triggered the validation layer
 *  message should be aborted. If this returns true, then the call is aborted with
 *  VK_ERROR_VALIDATION_FAILED_EXT error.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanBaseApplication::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void VulkanBaseApplication::cleanUp()
{
	if (enableValidationLayers) {
		vkutils::destroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
	}

	vkDestroyInstance(vulkanInstance, nullptr);
}