#include "Instance.h"

Instance::Instance()
{
#ifdef NDEBUG
	validationLayers.clear();
#endif

	if (!checkLayersSupport(validationLayers))
	{
		validationLayers.clear();
		LOGW("Validation layers not supported.");
	}

	validationEnabled = !validationLayers.empty();

	if (validationEnabled)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		LOGI("Validation layers enabled.");
	}

	LOGA(checkExtensionsSupport(extensions), "Required extensions not presented by instance.");

	createInstance();

	if (validationEnabled)
	{
		createDebugCallback();
	}
}

Instance::~Instance()
{
	if (callback)
	{
		vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	}
	vkDestroyInstance(instance, nullptr);
}

VkInstance Instance::get() const
{
	return instance;
}

std::vector<const char*> Instance::getLayers() const
{
	return validationLayers;
}

void Instance::createInstance()
{
	VkApplicationInfo appInfo{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"VulkanAndroid",
		VK_MAKE_VERSION(1, 0, 0),
		"No Engine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo createInfo{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&appInfo,
		uint32_t(validationLayers.size()),
		validationLayers.data(),
		uint32_t(extensions.size()),
		extensions.data()
	};

	CALL_VK(vkCreateInstance(&createInfo, nullptr, &instance));
	LOGI("Instance created.");
}

bool Instance::checkLayersSupport(const std::vector<const char*> &requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);  // get count

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // get layers

	std::set<std::string> requiredLayerSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto &layer : availableLayers)
	{
		requiredLayerSet.erase(layer.layerName);
	}

	// empty if all required layers are supported by instance
	return requiredLayerSet.empty();
}

bool Instance::checkExtensionsSupport(const std::vector<const char*> &requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);  // get count

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());  // get extensions

	std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto &layer : availableExtensions)
	{
		requiredExtensionSet.erase(layer.extensionName);
	}

	// empty if all required extensions are supported by instance
	return requiredExtensionSet.empty();
}

void Instance::createDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
		nullptr,
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
		validationLayerCallback,
		nullptr
	};

	CALL_VK(vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback));
}

VkResult Instance::vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugReportCallbackEXT *pCallback)
{
	const auto func = PFN_vkCreateDebugReportCallbackEXT(
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Instance::vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks *pAllocator)
{
	const auto func = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::validationLayerCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char *layerPrefix,
	const char *msg,
	void *userData)
{
    if (flags >= VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        LOGE("Validation layer: %s.", msg);
    }
    else if (flags >= VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        LOGW("Validation layer: %s.", msg);
    }
    else
    {
        LOGI("Validation layer: %s.", msg);
    }

	return false;
}
