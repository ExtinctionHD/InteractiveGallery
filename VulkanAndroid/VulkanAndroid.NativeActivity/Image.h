#pragma once
#include "SwapChainImage.h"

class Image : public SwapChainImage
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

	~Image();

	VkExtent3D getExtent() const;

	uint32_t getMipLevelCount() const;

	uint32_t getArrayLayerCount() const;

    VkSampleCountFlagBits getSampleCount() const;

	void transitLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) const;

	void updateData(std::vector<const void*>, uint32_t layersOffset, uint32_t pixelSize) const;

	void copyTo(Image *dstImage, VkExtent3D extent, VkImageSubresourceLayers subresourceLayers) const;

protected:
	Image() = default;

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

private:
	VkDeviceMemory memory;

	void allocateMemory();
};

