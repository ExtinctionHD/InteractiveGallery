#include "DescriptorPool.h"

DescriptorPool::DescriptorPool(Device *device, uint32_t bufferCount, uint32_t textureCount, uint32_t setCount) : device(device)
{
    const VkDescriptorPoolSize uniformBufferSize{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		bufferCount,
	};
    const VkDescriptorPoolSize textureSize{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		textureCount,
	};

	std::vector<VkDescriptorPoolSize> poolSizes{ uniformBufferSize, textureSize };

	VkDescriptorPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,										
		0,												
		setCount,										
		uint32_t(poolSizes.size()),						
		poolSizes.data(),								
	};

    CALL_VK(vkCreateDescriptorPool(device->get(), &createInfo, nullptr, &pool));
    LOGI("Descriptor pool created.");
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device->get(), pool, nullptr);
}

VkDescriptorSetLayout DescriptorPool::createDescriptorSetLayout(
	std::vector<VkShaderStageFlags> bufferShaderStages,
	std::vector<VkShaderStageFlags> textureShaderStages) const
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (size_t i = 0; i < bufferShaderStages.size(); i++)
	{
		VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{
			uint32_t(i),                        
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,                                
			bufferShaderStages[i],           
			nullptr                           
		};

		bindings.push_back(uniformBufferLayoutBinding);
	}

	for (size_t i = 0; i < textureShaderStages.size(); i++)
	{
		VkDescriptorSetLayoutBinding textureLayoutBinding{
			uint32_t(bufferShaderStages.size() + i),    
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
			1,                                         
			textureShaderStages[i],                   
			nullptr                                    
		};

		bindings.push_back(textureLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,											
		0,													
		uint32_t(bindings.size()),									
		bindings.data(),									
	};

	VkDescriptorSetLayout layout;
    CALL_VK(vkCreateDescriptorSetLayout(device->get(), &createInfo, nullptr, &layout));

	return layout;
}

void DescriptorPool::destroyDescriptorSetLayout(VkDescriptorSetLayout layout) const
{
    vkDestroyDescriptorSetLayout(device->get(), layout, nullptr);
}

VkDescriptorSet DescriptorPool::getDescriptorSet(VkDescriptorSetLayout layout) const
{
	VkDescriptorSetAllocateInfo allocateInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,										
		pool,											
		1,												
		&layout											
	};

	VkDescriptorSet set;
    CALL_VK(vkAllocateDescriptorSets(device->get(), &allocateInfo, &set));

	return set;
}

void DescriptorPool::updateDescriptorSet(
	VkDescriptorSet set, 
	std::vector<Buffer*> buffers,
	std::vector<TextureImage*> textures) const
{
	std::vector<VkWriteDescriptorSet> bufferWrites;
	std::vector<VkDescriptorBufferInfo> bufferInfos(buffers.size());

	for (size_t i = 0; i < buffers.size(); i++)
	{
		bufferInfos[i] = {
			buffers[i]->get(),
			0,
			buffers[i]->getSize()
		};

		VkWriteDescriptorSet bufferWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,							
			set,								
			uint32_t(i),									
			0,				
			1,		
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	
			nullptr,							
			&bufferInfos[i],					
			nullptr,							
		};

		bufferWrites.push_back(bufferWrite);
	}

	std::vector<VkWriteDescriptorSet> textureWrites;
	std::vector<VkDescriptorImageInfo> imagesInfo(textures.size());

	for (size_t i = 0; i < textures.size(); i++)
	{
		imagesInfo[i] = {
			textures[i]->getSampler(),				
			textures[i]->getView(),					
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		VkWriteDescriptorSet textureWrite{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,									
			set,										
			uint32_t(buffers.size() + i),						
			0,											
			1,											
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	
			&imagesInfo[i],								
			nullptr,									
			nullptr,									
		};

		textureWrites.push_back(textureWrite);
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites(bufferWrites.begin(), bufferWrites.end());
	descriptorWrites.insert(descriptorWrites.end(), textureWrites.begin(), textureWrites.end());

	vkUpdateDescriptorSets(device->get(), uint32_t(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorPool::freeDescriptorSets(std::vector<VkDescriptorSet> sets)
{
    vkFreeDescriptorSets(device->get(), pool, sets.size(), sets.data());
}
