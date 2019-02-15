#pragma once
#include "QueueFamilyIndices.h"
#include "SurfaceSupportDetails.h"

class Device
{
public:
	Device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> &requiredLayers);

	~Device();

	VkDevice get() const;

	VkQueue getGraphicsQueue() const;

	VkQueue getPresentQueue() const;

	VkCommandPool getCommandPool() const;

	VkFormatProperties getFormatProperties(VkFormat format) const;

    void updateSurface(VkSurfaceKHR surface);

	// returns index of memory type with such properties (for this physical device)
	uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	// returns first supported format (for this physical device)
	VkFormat findSupportedFormat(
		std::vector<VkFormat> requestedFormats,
		VkImageTiling tiling,
		VkFormatFeatureFlags features) const;

	SurfaceSupportDetails getSurfaceSupportDetails() const;

	QueueFamilyIndices getQueueFamilyIndices() const;

    VkSampleCountFlagBits getMaxSampleCount() const;

	// returns command buffer to write one time commands
	VkCommandBuffer beginOneTimeCommands() const;

	// ends command buffer and submit it to graphics queue
	void endOneTimeCommands(VkCommandBuffer commandBuffer) const;

private:
	const std::vector<const char*> EXTENSIONS{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDevice device;

	VkPhysicalDevice physicalDevice{};

	VkSurfaceKHR surface;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkCommandPool commandPool;

	void pickPhysicalDevice(VkInstance instance, const std::vector<const char*> &layers);

	bool physicalDeviceSuitable(
		VkPhysicalDevice device,
		const std::vector<const char*> &requiredLayers,
		const std::vector<const char*> &requiredExtensions) const;

	static bool checkDeviceLayerSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredLayers);

	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions);

	void createDevice(const std::vector<const char*> &layers);

	void createCommandPool();
};
