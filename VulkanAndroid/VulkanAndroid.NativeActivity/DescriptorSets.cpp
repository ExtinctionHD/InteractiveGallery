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

void DescriptorSets::pushDescriptorSet(DescriptorInfos descriptorInfos)
{
    sets.push_back(pool->getDescriptorSet(layout));
    pool->updateDescriptorSet(sets.back(), descriptorInfos);
}

void DescriptorSets::updateDescriptorSet(uint32_t index, DescriptorInfos descriptorInfos)
{
    pool->updateDescriptorSet(sets[index], descriptorInfos);
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
