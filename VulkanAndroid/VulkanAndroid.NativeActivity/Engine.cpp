#include "Engine.h"
#include "Vertex.h"

Engine::Engine(ANativeWindow *window)
{
	InitVulkan();

    const VkExtent2D extent{
        uint32_t(ANativeWindow_getWidth(window)),
        uint32_t(ANativeWindow_getHeight(window))
    };

    LOGD("Window extent: %d x %d.", extent.width, extent.height);

	instance = new Instance();
	surface = new Surface(instance->get(), window);
	device = new Device(instance->get(), surface->get(), instance->getLayers());
    swapChain = new SwapChain(device, surface->get(), extent);
    descriptorPool = new DescriptorPool(device, 1, 1, 1);

    mainRenderPass = new MainRenderPass(device, swapChain);
    mainRenderPass->create();

    const std::string shadersPath = "shaders/Main/";
    const std::vector<std::shared_ptr<ShaderModule>> shaders{
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    graphicsPipeline = new GraphicsPipeline(
        device,
        mainRenderPass,
        {},
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true);

    imageAvailableSemaphore = createSemaphore();
    passFinishedSemaphores.resize(RenderPass::LAST + 1, createSemaphore());

    initGraphicsCommands();

    LOGI("Engine initialized.");
}

Engine::~Engine()
{
    delete swapChain;
	delete device;
	delete surface;
	delete instance;
}

void Engine::drawFrame()
{
    uint32_t imageIndex;

    CALL_VK(vkAcquireNextImageKHR(
        device->get(), 
        swapChain->get(), 
        UINT64_MAX, 
        imageAvailableSemaphore, 
        nullptr, 
        &imageIndex));

    std::vector<VkSemaphore> waitSemaphores = {  imageAvailableSemaphore };
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::vector<VkSemaphore> signalSemaphores = { passFinishedSemaphores[RenderPass::MAIN] };
    VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        uint32_t(waitSemaphores.size()),
        waitSemaphores.data(),
        waitStages.data(),
        1,
        &graphicsCommands[imageIndex],
        uint32_t(signalSemaphores.size()),
        signalSemaphores.data(),
    };
    CALL_VK(vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, nullptr));

    std::vector<VkSwapchainKHR> swapChains{ swapChain->get() };
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        uint32_t(signalSemaphores.size()),
        signalSemaphores.data(),
        uint32_t(swapChains.size()),
        swapChains.data(),
        &imageIndex,
        nullptr,
    };
    CALL_VK(vkQueuePresentKHR(device->getPresentQueue(), &presentInfo));
}

VkSemaphore Engine::createSemaphore() const
{
    VkSemaphore semaphore;

    VkSemaphoreCreateInfo createInfo{
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0,
    };

    CALL_VK(vkCreateSemaphore(device->get(), &createInfo, nullptr, &semaphore));

    return semaphore;
}

void Engine::initGraphicsCommands()
{
    const VkCommandPool commandPool = device->getCommandPool();
    const uint32_t count = swapChain->getImageCount();

    if (!graphicsCommands.empty())
    {
        vkFreeCommandBuffers(device->get(), commandPool, uint32_t(graphicsCommands.size()), graphicsCommands.data());
    }

    graphicsCommands.resize(count);

    VkCommandBufferAllocateInfo allocInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        count,
    };

    CALL_VK(vkAllocateCommandBuffers(device->get(), &allocInfo, graphicsCommands.data()));

    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        nullptr,
    };

    for (uint32_t i = 0; i < count; i++)
    {
        CALL_VK(vkBeginCommandBuffer(graphicsCommands[i], &beginInfo));

        const VkRect2D renderArea{
            { 0, 0 },
            mainRenderPass->getExtent()
        };

        auto clearValues = mainRenderPass->getClearValues();

        VkRenderPassBeginInfo renderPassBeginInfo{
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            mainRenderPass->get(),
            mainRenderPass->getFramebuffers()[i],
            renderArea,
            uint32_t(clearValues.size()),
            clearValues.data()
        };

        vkCmdBeginRenderPass(graphicsCommands[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // TODO rendering here

        vkCmdEndRenderPass(graphicsCommands[i]);

        CALL_VK(vkEndCommandBuffer(graphicsCommands[i]));
    }

    LOGI("Graphics commands initialized.");
}
