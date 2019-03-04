#pragma once

#include "Device.h"
#include "IDescriptorSource.h"

class Image : public IDescriptorSource
{
public:
	Image(
		Device* device,
		VkImageCreateFlags flags,
		VkFormat format,
		VkExtent3D extent,
		uint32_t mipLevels,
		uint32_t arrayLayers,
		VkSampleCountFlagBits sampleCount,
		VkImageUsageFlags usage,
		VkImageAspectFlags aspectFlags,
		bool cubeMap);

    // For SwapChain images
    Image(Device *device, VkImage image, VkFormat format);

	virtual ~Image();

    VkImage get() const;

    VkImageView getView() const;

    VkFormat getFormat() const;

	VkExtent3D getExtent() const;

	uint32_t getMipLevelCount() const;

	uint32_t getArrayLayerCount() const;

    VkSampleCountFlagBits getSampleCount() const;

	void transitLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) const;

	void updateData(std::vector<const void*>, uint32_t layersOffset, uint32_t pixelSize) const;

	void copyTo(Image *dstImage, VkExtent3D extent, VkImageSubresourceLayers subresourceLayers) const;

protected:
	Image() = default;

    Device *device;

    VkImage image;

    VkImageView view;

    VkFormat format;

	VkExtent3D extent;

	uint32_t mipLevels;

	uint32_t arrayLayers;

    VkSampleCountFlagBits sampleCount;

	void createThisImage(
		Device* device,
		VkImageCreateFlags flags,
		VkFormat format,
		VkExtent3D extent,
		uint32_t mipLevels,
		uint32_t arrayLayers,
		VkSampleCountFlagBits sampleCount,
		VkImageUsageFlags usage,
		VkImageAspectFlags aspectFlags,
		bool cubeMap);

    void createView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType);

private:
	VkDeviceMemory memory;

	void allocateMemory();
};

