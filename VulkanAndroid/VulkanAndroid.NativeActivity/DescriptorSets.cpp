#include "DescriptorSets.h"

DescriptorSets::DescriptorSets(DescriptorPool *pool, DescriptorShaderStages descriptorShaderStages)
    : pool(pool),
      layout(pool->createDescriptorSetLayout(descriptorShaderStages))
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

void DescriptorSets::pushDescriptorSet(DescriptorSources descriptorSources)
{
    sets.push_back(pool->getDescriptorSet(layout));
    pool->updateDescriptorSet(sets.back(), descriptorSources);
}

void DescriptorSets::updateDescriptorSet(uint32_t index, DescriptorSources descriptorSources)
{
    pool->updateDescriptorSet(sets[index], descriptorSources);
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
