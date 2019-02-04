#pragma once
#define VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan_wrapper.h"
#include <vector>

class Instance
{
public:
	Instance();
	~Instance();

	VkInstance get() const;

private:
    const std::vector<const char*> REQUIRED_LAYERS{  
#ifndef NDEBUG
		"VK_LAYER_LUNARG_standard_validation",
#endif
	};
	const std::vector<const char*> REQUIRED_EXTENSIONS{
        VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#ifndef NDEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
	};

	VkInstance instance{};

	VkDebugReportCallbackEXT callback{};

	void createInstance();

	static bool checkLayersSupport(const std::vector<const char*> &requiredLayers);

	static bool checkExtensionsSupport(const std::vector<const char*> &requiredExtensions);

	void createDebugCallback();

	// functions from extensions (EXT) must be obtained before use
	static VkResult vkCreateDebugReportCallbackEXT(
		VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugReportCallbackEXT *pCallback);

	static void vkDestroyDebugReportCallbackEXT(
		VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks *pAllocator);

	// log messages from vulkan validation layers
	static VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char *layerPrefix,
		const char *msg,
		void *userData);
};

