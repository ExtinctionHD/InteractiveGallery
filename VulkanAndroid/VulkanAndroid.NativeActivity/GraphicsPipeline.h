#pragma once
#include "RenderPass.h"
#include "ShaderModule.h"
#include <memory>

class GraphicsPipeline
{
public:
	GraphicsPipeline(
		Device *device,
		RenderPass *renderPass,
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        const std::vector<VkPushConstantRange> &pushConstantRanges,
        std::vector<std::shared_ptr<ShaderModule>> shaderModules,
        std::vector<VkVertexInputBindingDescription> bindingDescriptions,
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
		bool blendEnable);

	~GraphicsPipeline();

	VkPipeline get() const;

	VkPipelineLayout getLayout() const;

	void recreate();

private:
	Device *device;

	RenderPass *renderPass;

	VkPipeline pipeline;

	VkPipelineLayout layout;

	std::vector<std::shared_ptr<ShaderModule>> shaderModules;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	bool blendEnable;

	void createLayout(
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
		const std::vector<VkPushConstantRange> &pushConstantRanges);

	void createPipeline();
};

