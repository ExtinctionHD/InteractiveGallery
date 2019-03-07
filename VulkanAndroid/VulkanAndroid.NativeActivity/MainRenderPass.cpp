#include "MainRenderPass.h"

MainRenderPass::MainRenderPass(Device *device, VkExtent2D attachmentExtent, VkSampleCountFlagBits sampleCount)
    : RenderPass(device, attachmentExtent, sampleCount)
{
    
}

uint32_t MainRenderPass::getColorAttachmentCount() const
{
    return 1;
}

std::vector<VkClearValue> MainRenderPass::getClearValues() const
{
    VkClearValue colorClearValue;
    colorClearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkClearValue depthClearValue;
    depthClearValue.depthStencil = { 1.0f, 0 };

    return { colorClearValue, depthClearValue };
}

TextureImage* MainRenderPass::getColorTexture() const
{
    return colorTexture.get();
}

void MainRenderPass::createAttachments()
{
    const VkFormat colorImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

    const VkExtent3D attachmentExtent{
        extent.width,
        extent.height,
        1
    };

    VkImageSubresourceRange subresourceRange{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1,
    };

    LOGA(device->getFormatProperties(colorImageFormat).optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    colorTexture = std::make_shared<TextureImage>(
        device,
        0,
        colorImageFormat,
        attachmentExtent,
        subresourceRange.levelCount,
        subresourceRange.layerCount,
        sampleCount,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        subresourceRange.aspectMask,
        false,
        VK_FILTER_NEAREST,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    colorTexture->transitLayout(
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        subresourceRange);

    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthImage = std::make_shared<Image>(
        device,
        0,
        depthAttachmentFormat,
        attachmentExtent,
        subresourceRange.levelCount,
        subresourceRange.layerCount,
        sampleCount,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        subresourceRange.aspectMask,
        false);
    depthImage->transitLayout(
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        subresourceRange);

	attachments = { colorTexture, depthImage };
}

void MainRenderPass::createRenderPass()
{
	// description of attachments

    const VkAttachmentDescription colorAttachmentDesc{
		0,								
        colorTexture->getFormat(),
        colorTexture->getSampleCount(),
		VK_ATTACHMENT_LOAD_OP_CLEAR,		         
		VK_ATTACHMENT_STORE_OP_STORE,		     
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,	     
		VK_ATTACHMENT_STORE_OP_DONT_CARE,	     
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

    const VkAttachmentDescription depthAttachmentDesc{
		0,										
		depthImage->getFormat(),						
		depthImage->getSampleCount(),					
		VK_ATTACHMENT_LOAD_OP_CLEAR,						 
		VK_ATTACHMENT_STORE_OP_DONT_CARE,				
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,				
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	std::vector<VkAttachmentDescription> attachmentDescriptions{
		colorAttachmentDesc,
		depthAttachmentDesc,
	};

	// references to attachments

	VkAttachmentReference colorAttachmentRef{
		0,							
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference depthAttachmentRef{
		1,									
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// subpass and it dependencies (contain references)

	VkSubpassDescription subpass{
		0,						
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,								
		nullptr,						
		1,								
		&colorAttachmentRef,			
		nullptr,			
		&depthAttachmentRef,			
		0,								
		nullptr							
	};

    const VkSubpassDependency inputDependency{
		VK_SUBPASS_EXTERNAL,							
		0,												
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	
		VK_ACCESS_MEMORY_READ_BIT,						
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	        
		VK_DEPENDENCY_BY_REGION_BIT,                    
	};

    const VkSubpassDependency outputDependency{
		0,									
		VK_SUBPASS_EXTERNAL,							
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	        
		VK_ACCESS_MEMORY_READ_BIT,						
		VK_DEPENDENCY_BY_REGION_BIT,					
	};

	std::vector<VkSubpassDependency> dependencies{
		inputDependency,
		outputDependency
	};

	// render pass (contain descriptions)

	VkRenderPassCreateInfo createInfo{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,	
		nullptr,									
		0,											
		uint32_t(attachmentDescriptions.size()),
		attachmentDescriptions.data(),				
		1,											
		&subpass,									
		uint32_t(dependencies.size()),						
		dependencies.data(),						
	};

    CALL_VK(vkCreateRenderPass(device->get(), &createInfo, nullptr, &renderPass));
    LOGI("Main render pass created.");
}

void MainRenderPass::createFramebuffers()
{
    addFramebuffer({ colorTexture->getView(), depthImage->getView(), });
}
