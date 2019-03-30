#pragma once
#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class TextureImage : public Image
{
public:
    TextureImage(
        Device *device,
        const std::vector<std::vector<uint8_t>> &buffers,
        bool mipLevels,
        bool cubeMap);

    TextureImage(
        Device* device,
        VkImageCreateFlags flags,
        VkFormat format,
        VkExtent3D extent,
        uint32_t mipLevels,
        uint32_t arrayLayers,
        VkSampleCountFlagBits sampleCount,
        VkImageUsageFlags usage,
        bool cubeMap);

	~TextureImage();

	VkSampler getSampler(uint32_t index) const;

    DescriptorInfo getCombineSamplerInfo(uint32_t samplerIndex = 0, uint32_t viewIndex = 0) const;

    void pushSampler(VkFilter filter, VkSamplerAddressMode addressMode);

protected:
	std::vector<VkSampler> samplers;

	stbi_uc* loadPixels(const std::vector<uint8_t> &buffer);
};

