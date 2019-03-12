#include "TextureImage.h"
#include "utils.h"
#include "AssetManager.h"

TextureImage::TextureImage(
	Device *device,
	const std::vector<std::string> &paths,
	bool cubeMap,
	VkFilter filter,
	VkSamplerAddressMode samplerAddressMode)
{
    extent = VkExtent3D{0, 0, 1};

	std::vector<const void*> pixels(paths.size());
	for (uint32_t i = 0; i < paths.size(); i++)
	{
		pixels[i] = loadPixels(paths[i]);
	}

	mipLevels = math::ceilLog2(std::max(extent.width, extent.height));
	mipLevels = mipLevels > 0 ? mipLevels : 1;

	createThisImage(
		device,
        0,
        VK_FORMAT_R8G8B8A8_UNORM,
		extent,
        mipLevels,
        paths.size(),
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
		cubeMap);

	updateData(pixels, 0, STBI_rgb_alpha);

    for (auto arrayLayerPixels : pixels)
    {
		stbi_image_free(const_cast<void*>(arrayLayerPixels));
    }

	generateMipmaps(VK_IMAGE_ASPECT_COLOR_BIT, filter);

	createSampler(filter, samplerAddressMode);
}

TextureImage::TextureImage(
    Device* device,
    VkImageCreateFlags flags,
    VkFormat format,
    VkExtent3D extent,
    uint32_t mipLevels,
    uint32_t arrayLayers,
    VkSampleCountFlagBits sampleCount,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspectFlags,
    bool cubeMap,
    VkFilter filter,
    VkSamplerAddressMode samplerAddressMode)
	: Image(
        device,
        flags,
        format,
        extent,
        mipLevels,
        arrayLayers,
        sampleCount,
        usage | VK_IMAGE_USAGE_SAMPLED_BIT,
        aspectFlags,
        cubeMap)
{
	createSampler(filter, samplerAddressMode);
}

TextureImage::~TextureImage()
{
	vkDestroySampler(device->get(), sampler, nullptr);
}

VkSampler TextureImage::getSampler() const
{
	return sampler;
}

DescriptorInfo TextureImage::getCombineSamplerInfo() const
{
    DescriptorInfo info;
    info.image = VkDescriptorImageInfo{
        sampler,
        view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    return info;
}

stbi_uc* TextureImage::loadPixels(const std::string &path)
{
    std::vector<stbi_uc> buffer = AssetManager::getBytes(path);

    int width, height;

    stbi_uc *pixels = stbi_load_from_memory(
        buffer.data(), 
        buffer.size(), 
        &width, 
        &height, 
        nullptr,
        STBI_rgb_alpha);

    LOGA(pixels);

    LOGI("Texture [%s] loaded, width: %d, height %d.", path.c_str(), width, height);

    if (extent.width && extent.height)
    {
        const bool sameExtent = extent.width == uint32_t(width) && extent.height == uint32_t(height);
        LOGA(sameExtent);
    }
    else
    {
        extent.width = uint32_t(width);
        extent.height = uint32_t(height);
    }

	return pixels;
}

void TextureImage::generateMipmaps(VkImageAspectFlags aspectFlags, VkFilter filter)
{
	const auto featureFlags = device->getFormatProperties(format).optimalTilingFeatures;
	LOGA(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

	VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

    VkImageSubresourceRange subresourceRange{
        aspectFlags,
        0,
        1,
        0,
        arrayLayers
    };

	int32_t mipWidth = extent.width;
	int32_t mipHeight = extent.height;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		// transit current miplevel layout to TRANSFER_SRC
		subresourceRange.baseMipLevel = i - 1;
        memoryBarrier(
            commandBuffer,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            subresourceRange);

        const VkImageSubresourceLayers srcSubresource{
            aspectFlags,
            i - 1,
            0,
            arrayLayers,
        };

        const VkImageSubresourceLayers dstSubresource{
            aspectFlags,
            i,
            0,
            arrayLayers,
        };

        blitTo(
            commandBuffer,
            this,
            srcSubresource,
            dstSubresource,
            { {
                VkOffset3D{ 0, 0, 0 },
                VkOffset3D{ mipWidth, mipHeight, 1 }
            } },
            { {
                VkOffset3D{ 0, 0, 0 },
                VkOffset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 }
            } },
            VK_FILTER_LINEAR);

		// transit current miplevel layout to SHADER_READ_ONLY
        memoryBarrier(
            commandBuffer,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            subresourceRange);

		// next miplevel scale
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// transit last miplevel layout to SHADER_READ_ONLY_OPTIMAL
	subresourceRange.baseMipLevel = mipLevels - 1;
    memoryBarrier(
        commandBuffer,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        subresourceRange);

	device->endOneTimeCommands(commandBuffer);
}

void TextureImage::createSampler(VkFilter filter, VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		nullptr,								
		0,										
		filter,									
		filter,									
		VK_SAMPLER_MIPMAP_MODE_LINEAR,			
		addressMode,			              
		addressMode,			              
		addressMode,			              
		0,										
		false,								
		0.0f,									
		false,								
		VK_COMPARE_OP_ALWAYS,					
		0,										
		float(mipLevels),						
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,		
		false,								
	};
	if (filter == VK_FILTER_NEAREST)
	{
		createInfo.anisotropyEnable = false;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}

    CALL_VK(vkCreateSampler(device->get(), &createInfo, nullptr, &sampler));
}
