#include "Image.h"
#include "StagingBuffer.h"

Image::Image(
	Device *device,
	VkImageCreateFlags flags,
	VkFormat format,
	VkExtent3D extent,
	uint32_t mipLevels,
	uint32_t arrayLayers,
	VkSampleCountFlagBits sampleCount,
	VkImageUsageFlags usage,
	VkImageAspectFlags aspectFlags,
	bool cubeMap)
{
	createThisImage(
		device,
		flags,
		format,
		extent,
		mipLevels,
		arrayLayers,
		sampleCount,
		usage,
		aspectFlags,
		cubeMap);
}

Image::~Image()
{
	vkDestroyImage(device->get(), image, nullptr);
	vkFreeMemory(device->get(), memory, nullptr);
}

VkExtent3D Image::getExtent() const
{
	return extent;
}

VkSampleCountFlagBits Image::getSampleCount() const
{
	return sampleCount;
}

uint32_t Image::getMipLevelCount() const
{
	return mipLevels;
}

uint32_t Image::getArrayLayerCount() const
{
	return arrayLayers;
}

void Image::transitLayout(
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageSubresourceRange subresourceRange) const
{
	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

	VkImageMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,								
		0,										
		0,										
		oldLayout,								
		newLayout,								
		VK_QUEUE_FAMILY_IGNORED,				
		VK_QUEUE_FAMILY_IGNORED,				
		image,									
		subresourceRange,						
	};

	VkPipelineStageFlags sourceStage{};
	VkPipelineStageFlags destinationStage{};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		FATAL("Unsupported image layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	device->endOneTimeCommands(commandBuffer);
}

void Image::updateData(std::vector<const void*> data, uint32_t layersOffset, uint32_t pixelSize) const
{
	const auto updatedLayers = uint32_t(data.size());

    LOGA(layersOffset + updatedLayers <= arrayLayers);

	const VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		mipLevels,
		layersOffset,
		updatedLayers
	};
	const VkDeviceSize layerSize = extent.width * extent.height * pixelSize;

	StagingBuffer stagingBuffer(device, layerSize * updatedLayers);
	for (uint32_t i = 0; i < updatedLayers; i++)
	{
		stagingBuffer.updateData(data[i], i * layerSize,layerSize);
	}

	std::vector<VkBufferImageCopy> regions(updatedLayers);
	for (uint32_t i = 0; i < updatedLayers; i++)
	{
		regions[i] = VkBufferImageCopy{
			i * layerSize,
			0,
			0,
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				layersOffset + i,
				1
			},
			{ 0, 0, 0 },
			{ extent.width, extent.height, 1 }
		};
	}

	// before copying the layout of the image must be TRANSFER_DST
	transitLayout(
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	stagingBuffer.copyToImage(image, regions);
}

void Image::copyTo(Image *dstImage, VkExtent3D extent, VkImageSubresourceLayers subresourceLayers) const
{
	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

	VkImageCopy region{
		subresourceLayers,	
		{ 0, 0, 0},
		subresourceLayers,
		{ 0, 0, 0 },
		extent	
	};

	vkCmdCopyImage(
		commandBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImage->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	device->endOneTimeCommands(commandBuffer);
}

void Image::createThisImage(
	Device* device,
	VkImageCreateFlags flags,
	VkFormat format,
	VkExtent3D extent,
	uint32_t mipLevels,
	uint32_t arrayLayers,
	VkSampleCountFlagBits sampleCount,
	VkImageUsageFlags usage,
	VkImageAspectFlags aspectFlags,
	bool cubeMap)
{
	this->device = device;
    this->format = format;
	this->extent = extent;
	this->mipLevels = mipLevels;
	this->arrayLayers = arrayLayers;
    this->sampleCount = sampleCount;

	VkImageType imageType = VK_IMAGE_TYPE_1D;
	VkImageViewType viewType = arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	if (extent.height > 0)
	{
		imageType = VK_IMAGE_TYPE_2D;
		viewType = VK_IMAGE_VIEW_TYPE_2D;

		if (cubeMap)
		{
			LOGA(arrayLayers >= 6);

			flags = flags | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else if (arrayLayers > 1)
		{
			viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
	}

	VkImageCreateInfo imageInfo{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		flags,
		imageType,
		format,
		extent,
		mipLevels,
		arrayLayers,
		sampleCount,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	CALL_VK(vkCreateImage(device->get(), &imageInfo, nullptr, &image));

	allocateMemory();

	vkBindImageMemory(device->get(), image, memory, 0);

	const VkImageSubresourceRange subresourceRange{
		aspectFlags,
		0,
		mipLevels,
		0,
		arrayLayers
	};

	createView(subresourceRange, viewType);
}

void Image::allocateMemory()
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->get(), image, &memRequirements);

    const uint32_t memoryTypeIndex = device->findMemoryTypeIndex(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,			
		memRequirements.size,	
		memoryTypeIndex,		
	};

    CALL_VK(vkAllocateMemory(device->get(), &allocInfo, nullptr, &memory));
}
