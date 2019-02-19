#pragma once
#include "DescriptorPool.h"

class DescriptorSets
{
public:
    DescriptorSets(
        DescriptorPool *pool,
        std::vector<VkShaderStageFlags> bufferShaderStages,
        std::vector<VkShaderStageFlags> textureShaderStages);

    ~DescriptorSets();

    VkDescriptorSetLayout getLayout() const;

    VkDescriptorSet getDescriptorSet(uint32_t index) const;

    void pushDescriptorSet(std::vector<Buffer*> buffers, std::vector<TextureImage*> textures);

    void updateDescriptorSet(uint32_t index, std::vector<Buffer*> buffers, std::vector<TextureImage*> textures);

    void removeDescriptorSets(uint32_t index, uint32_t count);

private:
    DescriptorPool *pool;

    VkDescriptorSetLayout layout;

    std::vector<VkDescriptorSet> sets;
};

