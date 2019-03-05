#include "Device.h"

Device::Device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> &requiredLayers) : surface(surface)
{
	pickPhysicalDevice(instance, requiredLayers);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    LOGD("Physical device: %s.", properties.deviceName);
    LOGD("Maximum sample count: %d.", getMaxSampleCount());

	createDevice(requiredLayers);
	createCommandPool();
}

Device::~Device()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

VkDevice Device::get() const
{
	return device;
}

VkQueue Device::getGraphicsQueue() const
{
	return graphicsQueue;
}

VkQueue Device::getComputeQueue() const
{
    return computeQueue;
}

VkQueue Device::getPresentQueue() const
{
	return presentQueue;
}

VkCommandPool Device::getCommandPool() const
{
	return commandPool;
}

VkFormatProperties Device::getFormatProperties(VkFormat format) const
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	return formatProperties;
}

void Device::updateSurface(VkSurfaceKHR surface)
{
    this->surface = surface;
}

uint32_t Device::findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	FATAL("Failed to find suitable memory type");
	return uint32_t{};
}

VkFormat Device::findSupportedFormat(std::vector<VkFormat> requestedFormats, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	for (auto format : requestedFormats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}

		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	FATAL("Failed to find suitable image format");
	return VkFormat{};
}

SurfaceSupportDetails Device::getSurfaceSupportDetails() const
{
	return { physicalDevice, surface };
}

QueueFamilyIndices Device::getQueueFamilyIndices() const
{
	return { physicalDevice, surface };
}

VkSampleCountFlagBits Device::getMaxSampleCount() const
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    const VkSampleCountFlags counts = std::min(
        physicalDeviceProperties.limits.framebufferColorSampleCounts,
        physicalDeviceProperties.limits.framebufferDepthSampleCounts
    );

    // if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    // if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    // if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    // if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkCommandBuffer Device::beginOneTimeCommands() const
{
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1,
	};

	CALL_VK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr,
	};

	CALL_VK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

void Device::endOneTimeCommands(VkCommandBuffer commandBuffer) const
{
	CALL_VK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0,
		nullptr,
		nullptr,
		1,
		&commandBuffer,
		0,
		nullptr,
	};

	CALL_VK(vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE));

	vkQueueWaitIdle(computeQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Device::pickPhysicalDevice(VkInstance instance, const std::vector<const char*> &layers)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);  // get count

    LOGA(deviceCount);

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());  // get devices

	for (auto device : physicalDevices)
	{
		if (physicalDeviceSuitable(device, layers, EXTENSIONS))
		{
			physicalDevice = device;
		}
	}

    LOGA(physicalDevice);
}

bool Device::physicalDeviceSuitable(
	VkPhysicalDevice device,
	const std::vector<const char*> &requiredLayers,
	const std::vector<const char*> &requiredExtensions) const
{
	QueueFamilyIndices indices(device, surface);
	SurfaceSupportDetails details(device, surface);

	const bool layerSupport = checkDeviceLayerSupport(device, requiredLayers);
	const bool extensionSupport = checkDeviceExtensionSupport(device, requiredExtensions);

	return indices.completed() && details.suitable() && layerSupport && extensionSupport;
}


bool Device::checkDeviceLayerSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredLayers)
{
	uint32_t layerCount;
	vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);  // get count

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateDeviceLayerProperties(device, &layerCount, availableLayers.data());  // get layers

	std::set<std::string> requiredLayerSet(requiredLayers.begin(), requiredLayers.end());

	for (const auto &layer : availableLayers)
	{
		requiredLayerSet.erase(layer.layerName);
	}

	// empty if all required layers are supported by device
	return requiredLayerSet.empty();
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);  // get count

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());  // get extensions

	std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto &layer : availableExtensions)
	{
		requiredExtensionSet.erase(layer.extensionName);
	}

	// empty if all required extensions are supported by device
	return requiredExtensionSet.empty();
}

void Device::createDevice(const std::vector<const char*> &layers)
{
	QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices();

	std::set<uint32_t> uniqueQueueFamilyIndices{
		queueFamilyIndices.getGraphics(),
		queueFamilyIndices.getPresentation()
	};

	// info about each unique queue family
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (auto queueFamilyIndex : uniqueQueueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			uint32_t(queueFamilyIndex),
			1,
			&queuePriority
		};

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.sampleRateShading = true;

	VkDeviceCreateInfo deviceCreateInfo{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		uint32_t(queueCreateInfos.size()),
		queueCreateInfos.data(),
		uint32_t(layers.size()),
		layers.data(),
		uint32_t(EXTENSIONS.size()),
		EXTENSIONS.data(),
		&deviceFeatures
	};

	CALL_VK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));
	LOGI("Device created.");

	// save queue handlers
	vkGetDeviceQueue(device, queueFamilyIndices.getGraphics(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.getCompute(), 0, &computeQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.getPresentation(), 0, &presentQueue);
}

void Device::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		0,
		getQueueFamilyIndices().getGraphics()
	};

	CALL_VK(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));
}
