#pragma once

#include "Device.h"
#include <array>
#include "DescriptorInfo.h"

class Image
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
		bool cubeMap);

    // For SwapChain images
    Image(Device *device, VkImage image, VkFormat format, VkExtent3D extent);

	virtual ~Image();

    VkImage get() const;

    VkImageView getView(uint32_t index = 0) const;

    VkFormat getFormat() const;

	VkExtent3D getExtent() const;

	uint32_t getMipLevelCount() const;

	uint32_t getArrayLayerCount() const;

    VkSampleCountFlagBits getSampleCount() const;

    DescriptorInfo getStorageImageInfo(uint32_t viewIndex = 0) const;

    void pushView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType);

    void pushFullView(VkImageAspectFlags aspectFlags);

    void memoryBarrier(
        VkCommandBuffer commandBuffer,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange);

	void transitLayout(
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange);

	void updateData(std::vector<const void*>, uint32_t layersOffset, uint32_t pixelSize);

    void blitTo(
        VkCommandBuffer commandBuffer,
        Image *dstImage,
        VkImageSubresourceLayers srcSubresource,
        VkImageSubresourceLayers dstSubresource,
        std::array<VkOffset3D, 2> srcOffsets,
        std::array<VkOffset3D, 2> dstOffsets,
        VkFilter filter) const;

    void blitTo(
        Image *dstImage,
        VkImageSubresourceLayers srcSubresource,
        VkImageSubresourceLayers dstSubresource,
        std::array<VkOffset3D, 2> srcOffsets,
        std::array<VkOffset3D, 2> dstOffsets,
        VkFilter filter) const;

    void copyTo(
        VkCommandBuffer commandBuffer,
        Image *dstImage,
        VkImageSubresourceLayers srcSubresource,
        VkImageSubresourceLayers dstSubresource,
        VkExtent3D extent) const;

	void copyTo(
        Image *dstImage,
        VkImageSubresourceLayers srcSubresource,
        VkImageSubresourceLayers dstSubresource,
        VkExtent3D extent) const;

protected:
	Image();

    Device *device;

    VkImage image;

    VkFormat format;

	VkExtent3D extent;

	uint32_t mipLevels;

	uint32_t arrayLayers;

    VkSampleCountFlagBits sampleCount;

    std::vector<VkImageView> views;

	void createThisImage(
		Device* device,
		VkImageCreateFlags flags,
		VkFormat format,
		VkExtent3D extent,
		uint32_t mipLevels,
		uint32_t arrayLayers,
		VkSampleCountFlagBits sampleCount,
		VkImageUsageFlags usage,
		bool cubeMap);

private:
	VkDeviceMemory memory;

    bool swapChainImage;

    bool cubeMap;

	void allocateMemory();
};

