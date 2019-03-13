#include "DescriptorPool.h"
#include "Buffer.h"
#include "TextureImage.h"
#include <list>

DescriptorPool::DescriptorPool(Device *device, std::vector<VkDescriptorPoolSize> descriptorCount, uint32_t setCount)
    : device(device)
{
	VkDescriptorPoolCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,										
		0,												
		setCount,										
		uint32_t(descriptorCount.size()),
        descriptorCount.data(),
	};

    CALL_VK(vkCreateDescriptorPool(device->get(), &createInfo, nullptr, &pool));
    LOGI("Descriptor pool created.");
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device->get(), pool, nullptr);
}

VkDescriptorSetLayout DescriptorPool::createDescriptorSetLayout(DescriptorShaderStages descriptorShaderStages) const
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

    for (const auto &[descriptorType, shaderStages] : descriptorShaderStages)
    {
        for (auto shaderStage : shaderStages)
        {
            VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{
                uint32_t(bindings.size()),
                descriptorType,
                1,
                shaderStage,
                nullptr
            };

            bindings.push_back(uniformBufferLayoutBinding);
        }
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

void DescriptorPool::updateDescriptorSet(VkDescriptorSet set, DescriptorInfos descriptorInfos) const
{
    std::list<VkDescriptorImageInfo> imageInfos;
    std::list<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    for (const auto &[descriptorType, infos] : descriptorInfos)
    {
        switch (descriptorType)
        {
        // case VK_DESCRIPTOR_TYPE_SAMPLER: break;

        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            for (auto info : infos)
            {
                imageInfos.push_back(info.image);

                descriptorWrites.push_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    set,
                    uint32_t(descriptorWrites.size()),
                    0,
                    1,
                    descriptorType,
                    &imageInfos.back(),
                    nullptr,
                    nullptr
                });
            }
            break;

        // case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: break;

        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            for (auto info : infos)
            {
                imageInfos.push_back(info.image);

                descriptorWrites.push_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    set,
                    uint32_t(descriptorWrites.size()),
                    0,
                    1,
                    descriptorType,
                    &imageInfos.back(),
                    nullptr,
                    nullptr
                });
            }
            break;

        // case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: break;
        // case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            for (auto info : infos)
            {
                bufferInfos.push_back(info.buffer);

                descriptorWrites.push_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    set,
                    uint32_t(descriptorWrites.size()),
                    0,
                    1,
                    descriptorType,
                    nullptr,
                    &bufferInfos.back(),
                    nullptr
                });
            }
            break;

        // case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: break;
        // case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: break;
        // case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: break;
        // case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: break;
        // case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT: break;
        // case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: break;
        // case VK_DESCRIPTOR_TYPE_RANGE_SIZE: break;
        // case VK_DESCRIPTOR_TYPE_MAX_ENUM: break;

        default: 
            FATAL("Invalid descritor type: %d.", descriptorType);
        }
    }

	vkUpdateDescriptorSets(device->get(), uint32_t(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorPool::freeDescriptorSets(std::vector<VkDescriptorSet> sets)
{
    vkFreeDescriptorSets(device->get(), pool, sets.size(), sets.data());
}
