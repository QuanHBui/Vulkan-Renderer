#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

const uint32_t WIDTH = 800, HEIGHT = 600;

// List of validation layers to enable
const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

// List of required device extensions
const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
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
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, debugMessenger, pAllocator);
	}
}

/**
 * It is possible that queue families supporting drawing commands and the ones supporting presentation do not overlap.
 */
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;	// Drawing commands
	std::optional<uint32_t> presentFamily;	// Presenting commands

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

/**
 * 3 kinds of properties of a swap chain that we need to check:
 * 	(1) Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
 * 	(2) Surface formats (pixel format, color space)
 * 	(3) Available presentation modes
 */
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow *window;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;					// Connect between Vulkan and window system

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;						// Logical device handle

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;	// Handles of images in the swap chain
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	void initWindow()
	{
		glfwInit();
		// Tell GLFW to not create an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Disable resizing window because Vulkan needs some special care when doing this
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		// Create the actual window
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	/**
	 * Check if all of the layers in validationLayers exists in the availableLayers list
	 */
	bool checkValidationLayerSupport()
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

	/**
	 * Return the required list of extensions based on whether validation layers are enabled or not.
	 *  The debug messenger extension is conditionally added based on aformentioned condition.
	 */
	std::vector<const char *> getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Create a vector and fill it with content of glfwExtensions array. First parameter is the first element
		//  of the array, while the second parameter is the last element.
		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	/**
	 * Return a boolean that indicates if the Vulkan call that triggered the validation layer
	 *  message should be aborted. If this returns true, then the call is aborted with
	 *  VK_ERROR_VALIDATION_FAILED_EXT error.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
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

	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to set up debug messenger!");
		}
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;
		// Assign index to queue families that could be found
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT and also one that
		//  supports presenting to created window surface. Note that they can be the same one.
		int i = 0;
		for (const VkQueueFamilyProperties &queueFamily : queueFamilies) {
			// Look for drawing queue family
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			// Look for presenting queue family
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			++i;
		}

		return indices;
	}

	/**
	 * Fill in a struct VkApplicationInfo with information about the application.
	 *  VkApplicationInfo struct is optional but provides info to driver for optimization.
	 * A lot of information in Vulkan is passed through structs instead of function parameters.
	 * Must fill info to the second struct, VkInstanceCreateInfo, as it is NOT optional.
	 *  VkInstanceCreateInfo tells Vulkan driver which global extensions and validation layers to use.
	 */
	void createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("[ERROR] Validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Get info about required extensions
		std::vector<const char *> extensions = getRequiredExtensions();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		// Enable validation layers if debug mode
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create a Vulkan instance");
		}

		// // Get info about optional supported extensions
		// uint32_t extensionCount = 0;
		// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		// std::vector<VkExtensionProperties> extensions(extensionCount);
		// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// std::cout << "Available extensions:\n";
		// for (const auto &extension : extensions) {
		// 	std::cout << '\t' << extension.extensionName << '\n';
		// }

		// std::cout << "Required extensions:\n";
		// for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
		// 	std::cout << '\t' << glfwExtensions[i] << '\n';
		// }
	}

	/**
	 * Enumerate the extensions and check if all of the required extensions are amongst them.
	 * Typically, the availability of a presentation queue implies swap chain extension support;
	 *  it is still a good idea to be explicit.
	 */
	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// This is an interesting way to check off which required extension is available.
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const VkExtensionProperties &extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		// If requiredExtensions is empty, that means all the required extensions are available in the physical device.
		return requiredExtensions.empty();
	}

	bool checkAdequateSwapChain(VkPhysicalDevice device)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	/**
	 * Check if the graphic card is suitable for the operations we want to perform.
	 * Specifically, we are checking for graphic card type, geometry shader capability, and
	 *  queue family availability. Also check if the device can present images to the surface we created;
	 *  this is a queue-specific feature. Also check if the device support a certain extension and
	 *  adequate swap chain.
	 */
	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		// Query basic physical device properties: name, type, supported Vulkan version
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		// Query physical device supported features
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// Check for adequate swap chain
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			swapChainAdequate = checkAdequateSwapChain(device);
		}

		// Application needs dedicated GPU that support geometry shaders with certain queue family and extension
		return	(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
				deviceFeatures.geometryShader &&
				indices.isComplete() &&
				extensionsSupported &&
				swapChainAdequate;
	}

	/**
	 * Rate a particular GPU with a certain criteria. This implementation favors heavily dedicated GPU with geometry shader.
	 * Note that this function is similar to isDeviceSuitable function, but this will return a score instead of a boolean.
	 */
	int rateDeviceSuitability(VkPhysicalDevice device)
	{
		int score = 0;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		// Query basic device properties: name, type, supported Vulkan version
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		// Query supported features
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		// Make sure to take family queue into account
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (indices.isComplete()) {
			score += 10;
		}

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// If application can't function without geometry shader or a certain extension from the device
		if (!deviceFeatures.geometryShader || !extensionsSupported) {
			return 0;
		}

		// Check for swap chain, this might not be the best logic flow
		if (extensionsSupported && !checkAdequateSwapChain(device)) {
			return 0;
		}

		return score;
	}

	/**
	 * Look for and select a graphic card that supports the features we need. We can select
	 *  any number of graphic cards and use them simultaneously. This particular implementation
	 *  looks at all the available devices and scores each of them. The device with the highest score
	 *  will be picked. This allows for dedicated GPU to be picked if available, and the application
	 *  will fall back to integrated GPU if necessary.
	 */
	void pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// If no such graphic card with Vulkan support exists, no point to go further
		if (deviceCount == 0) {
			throw std::runtime_error("[ERROR] Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, VkPhysicalDevice> candidates;

		for (const VkPhysicalDevice &device : devices) {
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0) {
			physicalDevice = candidates.rbegin()->second;
		} else {
			throw std::runtime_error("[ERROR] Failed to find a suitable GPU!");
		}
	}

	void createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// We need to create multiple VkDeviceQueueCreateInfo structs to contain info for each queue from
		//  each family. We use set data structure because the 2 queue families can be the same.
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		// Assign priorities to queues to influence the scheduling of command buffer execution.
		// This is required even if there is only a single queue.
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Specify the set of device features that we'll be using
		VkPhysicalDeviceFeatures deviceFeatures{};

		// Create a logical device
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// These 2 fields enabledLayerCount and ppEnabledLayerNames are ignored by up-to-date implementation
		//  of Vulkan, but it's still a good idea to set them for backward compatibility.
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create logical device!");
		}

		// Queues are automatically created along with logical device; we just need to retrieve them.
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	/**
	 * Create a window surface object with GLFW library, to be platform agnostic. This process can be platform specific as well.
	 * The Vulkan tutorial has examples to implement this specifically for Windows and Linux. Both processes are similar in nature:
	 *  fill in the create info struct and then call a Vulkan function to create a window surface object.
	 */
	void createSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create window surface!");
		}
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	/**
	 * VK_FORMAT_B8G8R8A8_SRGB format stores B, G, R, and alpha channels with 8 bit unsinged integer, so 32-bit per pixel total.
	 * SRGB is the standard color space for images, e.g. textures, so we use both SRGB for both standard color format and color space.
	 */
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
	{
		for (const VkSurfaceFormatKHR &availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		// In most cases it's okay to settle with the first format that is specified
		return availableFormats[0];
	}

	/**
	 * Arguably the most important setting for swap chain since it sets the conditions for showing images to the
	 *  screen. There are 4 possible modes available in Vulkan. Only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available.
	 *  We, however, do try to look for the triple buffering mode VK_PRESENT_MODE_MAILBOX_KHR to avoid screen tearing
	 *  and fairly low latency.
	 */
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
	{
		for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	/**
	 * Swap extent is the resolution of the swap chain images. Almost always exactly equal to resolution of the
	 *  of the window that we're drawing to. Typically, we can just use the global variables WIDTH and HEIGHT
	 *  to specify the swap chain resolution, but some window managers allow use to differ the width and height
	 *  of the window - think about when you resize the window when not in fullscreen.
	 *
	 * To indicate that the width and height of the window are not the same as WIDTH and HEIGHT, VkSurfaceCapabilitiesKHR
	 *  uses the maximum value of uint32_t. In which case, we pick the resolution that matches the window within
	 *  the minImageExtent and maxImageExtent bounds.
	 */
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			VkExtent2D actualExtent = {WIDTH, HEIGHT};

			actualExtent.width = std::max(	capabilities.minImageExtent.width,
											std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(	capabilities.minImageExtent.height,
											std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void createSwapChain()
	{
		// Should these info be cached somewhere so we don't need to query this info every time
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Decide on how many images we would like to have in the swap chain. It is recommended to
		//  use at least one more image than the minimum.
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		// Make sure we don't exceed the maximum. 0 indicates no maximum.
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Fill in the create info struct
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;								// Amount of layers each image consists of
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// What kind of operations we'll use the images
																		//  in the swap chain for. We are going to render directly to them.
																		//  To relate to OpenGL, they are being used as color attachment.

		// Specify how handle swap chain images will be used across multiple queue families.
		// Things would be a bit complicated when the graphics queue family and presentation queue family
		//  are different. Luckily, they are the same family for most hardware; therefore, we will be using exclusive mode.
		//  For that, we don't need to do anything extra.

		// Specify that we do not want any transformation applied to the images in the swap chain
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// Specify if we want to use alpha channel for blending with other windows in the window system.
		// We almost always want to ignore the alpha channel.
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;	// Clipping for better performance, we don't care about obscured pixels.

		// TODO: swap chain can be obsolete in the case of window resizing. A new swap chain must be created from
		//  scratch. At the moment, we are assuming that only 1 swap chain is ever created.

		// Actually create the swap chain
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create swap chain!");
		}

		// Retrieve the handles of images in the swap chain. They are cleaned up automatically when the swap chain is destroyed.
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); ++i) {
			// Fill in the create info struct for image view
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];

			// Specify how image data should be interpreted
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;	// Treat image as 2D texture
			createInfo.format = swapChainImageFormat;

			// Swizzle the color channels around. We stick to default mapping.
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			// Describe what the image's purpose is and which part of the image should be accessed
			// In this case, images are used as color targets without mipmapping levels or multiple layers.
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			// Actually create a basic image view object for every image in the swap chain
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("[ERROR] Failed to create image views!");
			}
		}
	}

	/**
	 * Render pass object tells Vulkan about the framebuffer attachments for the rendering process.
	 * Specify how many color and depth buffers will be used, and how many samples to use for each
	 *  of them and how their contents should be handled over the rendering operations.
	 */
	void createRenderPass()
	{
		// Have a single color buffer attachment represented by one of the images from the swap chain
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;				// No multiplesampling, only use 1 sample
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;			// Clear the color attachment before drawing new frame
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// Rendered contents will be stored in memory to be read later
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// We don't care what previous layout the image was in
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// We want the image to be ready for presentation using the swap chain after rendering

		// Every subpass references one or more attachments with VkAttachmentReference struct
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;	// Directly referenced "layout(location = 0) out vec4 outColor" directive in the fragment shader
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// We intend to use attachment as a color buffer

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create render pass!");
		}
	}

	/**
	 * Simple helper function to load in the SPIR-V bytecode generated from the shaders
	 */
	static std::vector<char> readFile(const std::string &filename)
	{
		// We read from the end of the file, indicated by std::ios::ate.
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("[ERROR] Failed to open file!");
		}

		// We read from end of file so that we can use the read position to determine the
		//  size of the file and allocate a buffer.
		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);

		// Seek back at the beginning of the file and read all of the bytes all of the bytes at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	/**
	 * SPIR-V bytecode must be wrapped in a VkShaderModule object before being passed to the
	 *  graphics pipeline.
	 */
	VkShaderModule createShaderModule(const std::vector<char> &code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		// Reinterpret cast from char pointer to an uint32_t pointer. Data stored in a std::vector
		//  default allocator already takes care of data alignment requirements of uint32_t.
		createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create shader module!");
		}

		return shaderModule;
	}

	/**
	 * The loading and linking of SPIR-V bytecode for execution on the GPU.
	 * Creates shader modules for vertex shader stage and fragment shader stage, then creates
	 *  the pipeline shader stages, finally assigns the shader stages to a pipeline stage.
	 */
	void createGraphicsPipeline()
	{
		std::vector<char> vertShaderCode = readFile("../resources/shaders/vert.spv");
		std::vector<char> fragShaderCode = readFile("../resources/shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";		// Function to invoke in the shader, a.k.a the entrypoint

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		// Indicate that no vertex data to load because we hard coded the vertex data directly in the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// What kind of geometry will be drawn from the vertices: point, line, line strip, triangle, triangle strip, etc.
		// Also, no primitive restart
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewport describes the region of the framebuffer that the output will be rendered to.
		// Almost always (0, 0) to (width, height)
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// Set the scissor rectangle to cover the entire framebuffer so the rasterizer doesn't discard anything
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;	// Can be a pointer to an array in the case of multiple viewports
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;		// Can be a pointer to an array in the case of multiple scissor rectangles

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;		// Want to discard fragments outside the near and far planes as opposed to them being clamped to the planes.
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		// Disable multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		// Configuration per attached framebuffer
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		// Global color blending settings
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		// Create pipeline layout object. Used to specify uniform values
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;	// In case you want to change some values during rendering, extremely limited
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	// Vulkan allows creation of new pipeline derived from existing pipeline
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("[ERROR] Failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); ++i) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("[ERROR] Failed to create framebuffer!");
			}
		}
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		for (VkFramebuffer framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		for (VkImageView &imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main()
{
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::exception &thrownException) {
		std::cerr << thrownException.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}