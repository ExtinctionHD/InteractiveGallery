#include "TextureImage.h"
#include "AssetManager.h"

TextureImage::TextureImage(
	Device *device,
	const std::vector<std::string> &paths,
	bool cubeMap)
{
    extent = VkExtent3D{0, 0, 1};

	std::vector<const void*> pixels(paths.size());
	for (uint32_t i = 0; i < paths.size(); i++)
	{
		pixels[i] = loadPixels(paths[i]);
	}

	createThisImage(
		device,
        0,
        VK_FORMAT_R8G8B8A8_UNORM,
		extent,
        calculateMipLevelCount(extent),
        paths.size(),
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		cubeMap);

	updateData(pixels, 0, STBI_rgb_alpha);

    for (auto arrayLayerPixels : pixels)
    {
		stbi_image_free(const_cast<void*>(arrayLayerPixels));
    }

	generateMipmaps(
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FILTER_LINEAR,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
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
    bool cubeMap)
	: Image(
        device,
        flags,
        format,
        extent,
        mipLevels,
        arrayLayers,
        sampleCount,
        usage | VK_IMAGE_USAGE_SAMPLED_BIT,
        cubeMap)
{

}

TextureImage::~TextureImage()
{
    for (auto sampler : samplers)
    {
        vkDestroySampler(device->get(), sampler, nullptr);
    }
}

VkSampler TextureImage::getSampler(uint32_t index = 0) const
{
	return samplers[index];
}

DescriptorInfo TextureImage::getCombineSamplerInfo(uint32_t samplerIndex, uint32_t viewIndex) const
{
    DescriptorInfo info;
    info.image = VkDescriptorImageInfo{
        samplers[samplerIndex],
        views[viewIndex],
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    return info;
}

void TextureImage::pushSampler(VkFilter filter, VkSamplerAddressMode addressMode)
{
    VkSampler sampler;

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

    samplers.push_back(sampler);
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
