#pragma once
#include "Device.h"
#include "Buffer.h"
#include "TextureImage.h"

class DescriptorPool
{
public:
	DescriptorPool(Device *device, uint32_t bufferCount, uint32_t textureCount, uint32_t setCount);

	~DescriptorPool();

	VkDescriptorSetLayout createDescriptorSetLayout(
		std::vector<VkShaderStageFlags> bufferShaderStages,
		std::vector<VkShaderStageFlags> textureShaderStages) const;

    void destroyDescriptorSetLayout(VkDescriptorSetLayout layout) const;

	VkDescriptorSet getDescriptorSet(VkDescriptorSetLayout layout) const;

	void updateDescriptorSet(
		VkDescriptorSet set,
		std::vector<Buffer*> buffers,
		std::vector<TextureImage*> textures) const;

    void freeDescriptorSets(std::vector<VkDescriptorSet> sets);

private:
	Device *device;

	VkDescriptorPool pool;
};

