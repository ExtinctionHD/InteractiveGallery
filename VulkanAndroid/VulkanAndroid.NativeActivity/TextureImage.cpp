#include "TextureImage.h"

TextureImage::TextureImage(
    Device *device,
    const std::vector<std::vector<uint8_t>> &buffers,
    bool mipLevels,
    bool cubeMap)
{
    extent = VkExtent3D{ 0, 0, 1 };

    std::vector<const void*> pixels;
    for (uint32_t i = 0; i < buffers.size(); i++)
    {
        stbi_uc *loadedPixels = loadPixels(buffers[i]);
        if (loadedPixels)
        {
            pixels.push_back(loadedPixels);
        }
        else
        {
            failedImages.insert(i);
        }
    }

    createThisImage(
        device,
        0,
        VK_FORMAT_R8G8B8A8_UNORM,
        extent,
        mipLevels ? calculateMipLevelCount(extent) : 1,
        pixels.size(),
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

std::set<uint32_t> TextureImage::getFailedImages() const
{
    return failedImages;
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

stbi_uc* TextureImage::loadPixels(const std::vector<uint8_t> &buffer)
{
    int width, height;

    stbi_uc *pixels = stbi_load_from_memory(
        buffer.data(),
        buffer.size(),
        &width,
        &height,
        nullptr,
        STBI_rgb_alpha);

    LOGA(pixels);

    if (extent.width && extent.height)
    {
        const bool sameExtent = extent.width == uint32_t(width) && extent.height == uint32_t(height);
        if (!sameExtent)
        {
            return nullptr;
        }
    }
    else
    {
        extent.width = uint32_t(width);
        extent.height = uint32_t(height);
    }

    return pixels;
}
