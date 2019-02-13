#include "RenderPass.h"

RenderPass::~RenderPass()
{
	clear();
}

VkRenderPass RenderPass::get() const
{
	return renderPass;
}

std::vector<VkFramebuffer> RenderPass::getFramebuffers() const
{
	return framebuffers;
}

VkExtent2D RenderPass::getExtent() const
{
	return extent;
}

std::vector<VkClearValue> RenderPass::getClearValues() const
{
	std::vector<VkClearValue> clearValues(getColorAttachmentCount());
    for(auto &clearValue : clearValues)
    {
        clearValue.color = { { 0.0f, 1.0f, 0.0f, 1.0f } };
    }
    if (getColorAttachmentCount() < attachments.size())
    {
        VkClearValue clearValue{};
        clearValue.depthStencil = { 1.0f, 0 };
        clearValues.push_back(clearValue);
    }

	return clearValues;
}

VkSampleCountFlagBits RenderPass::getSampleCount() const
{
    return sampleCount;
}

uint32_t RenderPass::getRenderCount() const
{
    return 1;
}

void RenderPass::create()
{
	createAttachments();
	createRenderPass();
	createFramebuffers();
}

void RenderPass::recreate(VkExtent2D newExtent)
{
	clear();
	extent = newExtent;
	create();
}

RenderPass::RenderPass(Device *device, VkExtent2D extent, VkSampleCountFlagBits sampleCount)
    : device(device), extent(extent), sampleCount(sampleCount)
{
    depthAttachmentFormat = device->findSupportedFormat(
        DEPTH_FORMATS,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void RenderPass::addFramebuffer(std::vector<VkImageView> imageViews)
{
	VkFramebufferCreateInfo createInfo{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,			
		0,					
		renderPass,			
		uint32_t(imageViews.size()),
		imageViews.data(),	
		extent.width,		
		extent.height,		
		1,					
	};

	VkFramebuffer framebuffer;

    CALL_VK(vkCreateFramebuffer(device->get(), &createInfo, nullptr, &framebuffer));

	framebuffers.push_back(framebuffer);
}

void RenderPass::clear()
{
	attachments.clear();

	for (auto framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(device->get(), framebuffer, nullptr);
	}
	framebuffers.clear();

	vkDestroyRenderPass(device->get(), renderPass, nullptr);
}
