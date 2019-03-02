#pragma once
#include "Device.h"
#include "Buffer.h"
#include "TextureImage.h"
#include <map>

typedef std::map<VkDescriptorType, std::vector<VkShaderStageFlags>> DescriptorShaderStages;

typedef std::map<VkDescriptorType, std::vector<void*>> DescriptorSources;

class DescriptorPool
{
public:
	DescriptorPool(Device *device, std::vector<VkDescriptorPoolSize> descriptorCount, uint32_t setCount);

	~DescriptorPool();

	VkDescriptorSetLayout createDescriptorSetLayout(DescriptorShaderStages descriptorShaderStages) const;

    void destroyDescriptorSetLayout(VkDescriptorSetLayout layout) const;

	VkDescriptorSet getDescriptorSet(VkDescriptorSetLayout layout) const;

	void updateDescriptorSet( VkDescriptorSet set, DescriptorSources descriptorSources) const;

    void freeDescriptorSets(std::vector<VkDescriptorSet> sets);

private:
	Device *device;

	VkDescriptorPool pool;
};

