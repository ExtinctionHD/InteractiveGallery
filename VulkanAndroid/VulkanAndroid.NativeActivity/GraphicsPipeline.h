#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include <memory>

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline(
		Device *device,
		RenderPass *renderPass,
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        std::vector<VkPushConstantRange> pushConstantRanges,
        std::vector<std::shared_ptr<ShaderModule>> shaderModules,
        std::vector<VkVertexInputBindingDescription> bindingDescriptions,
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
        bool depthTestEnabled,
		bool blendEnable);

	~GraphicsPipeline() = default;

private:
	RenderPass *renderPass;

	std::vector<std::shared_ptr<ShaderModule>> shaderModules;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    bool depthTestEnabled;

	bool blendEnable;

	void createPipeline() override;
};

