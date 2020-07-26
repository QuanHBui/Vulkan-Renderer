#include "VulkanUtils.h"

namespace vkutils
{
	/**
	 * Load function vkCreateDebugUtilsMessengerEXT to create VkDebugUtilsMessengerEXT object.
	 *  This is function is a proxy function that calls vkGetInstanceProcAddr first.
	 */
	VkResult createDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		PFN_vkCreateDebugUtilsMessengerEXT pFunc = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (pFunc) {
			return pFunc(instance, pCreateInfo, pAllocator, pDebugMessenger);
		} else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	/**
	 * Similar to createDebugUtilsMessengerEXT, this is a proxy function to load and call vkDestroyDebugUtilsMessengerEXT
	 *  to destroy our created VkDebugUtilsMessengerEXT object.
	 */
	void destroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks *pAllocator)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT pFunc = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (pFunc) {
			pFunc(instance, debugMessenger, pAllocator);
		}
	}
}