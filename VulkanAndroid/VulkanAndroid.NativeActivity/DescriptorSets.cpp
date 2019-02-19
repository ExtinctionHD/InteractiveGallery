#include "DescriptorSets.h"

DescriptorSets::DescriptorSets(
    DescriptorPool *pool,
    std::vector<VkShaderStageFlags> bufferShaderStages,
    std::vector<VkShaderStageFlags> textureShaderStages)
    : pool(pool),
      layout(pool->createDescriptorSetLayout(bufferShaderStages, textureShaderStages))
{
}

DescriptorSets::~DescriptorSets()
{
    pool->destroyDescriptorSetLayout(layout);
}

VkDescriptorSetLayout DescriptorSets::getLayout() const
{
    return layout;
}

VkDescriptorSet DescriptorSets::getDescriptorSet(uint32_t index) const
{
    return sets[index];
}

void DescriptorSets::pushDescriptorSet(std::vector<Buffer *> buffers, std::vector<TextureImage *> textures)
{
    sets.push_back(pool->getDescriptorSet(layout));
    pool->updateDescriptorSet(sets.back(), buffers, textures);
}

void DescriptorSets::updateDescriptorSet(
    uint32_t index,
    std::vector<Buffer *> buffers,
    std::vector<TextureImage *> textures)
{
    pool->updateDescriptorSet(sets[index], buffers, textures);
}

void DescriptorSets::removeDescriptorSets(uint32_t index, uint32_t count)
{
    std::vector<VkDescriptorSet> removingSets(count);
    for (uint32_t i = 0; i < count; i++)
    {
        removingSets[i] = sets[index + i];
    }

    pool->freeDescriptorSets(removingSets);

    sets.erase(sets.begin() + index, sets.begin() + index + count);
}
