#pragma once
#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class TextureImage : public Image
{
public:
	TextureImage(
		Device *device,
		const std::vector<std::string> &paths,
		bool cubeMap,
		VkFilter filter,
		VkSamplerAddressMode samplerAddressMode);

	TextureImage(
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
		VkSamplerAddressMode samplerAddressMode);

	~TextureImage();

	VkSampler getSampler() const;

protected:
	VkSampler sampler;

	stbi_uc* loadPixels(const std::string &path);

	void generateMipmaps(VkImageAspectFlags aspectFlags, VkFilter filter);

	void createSampler(VkFilter filter, VkSamplerAddressMode addressMode);
};

