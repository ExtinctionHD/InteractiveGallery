#pragma once

class Instance
{
public:
	Instance();

	~Instance();

	VkInstance get() const;

	std::vector<const char*> getLayers() const;

private:
    std::vector<const char*> validationLayers{  
		"VK_LAYER_LUNARG_standard_validation",
	};

	std::vector<const char*> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
	};

	bool validationEnabled;

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

