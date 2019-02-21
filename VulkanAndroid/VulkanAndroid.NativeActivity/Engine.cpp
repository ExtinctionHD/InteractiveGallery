#include "Engine.h"
#include "Vertex.h"
#include "utils.h"

Engine::Engine() : created(false), outdated(false)
{
	InitVulkan();

	instance = new Instance();
}

Engine::~Engine()
{
    destroy();

	delete instance;
}

bool Engine::create(ANativeWindow *window)
{
    if (created) return true;

    surface = new Surface(instance->get(), window);
    device = new Device(instance->get(), surface->get(), instance->getLayers());
    swapChain = new SwapChain(device, surface->get(), window::getExtent(window));
    scene = new Scene(device, swapChain->getExtent());
    descriptorPool = new DescriptorPool(device, Scene::BUFFER_COUNT, Scene::TEXTURE_COUNT, COUNT);
    mainRenderPass = new MainRenderPass(device, swapChain);
    mainRenderPass->create();

    initDescriptorSets();
    createEarthPipeline();

    imageAvailableSemaphore = createSemaphore();
    passFinishedSemaphores.resize(RenderPass::COUNT, createSemaphore());
    initGraphicsCommands();

    created = true;
    LOGI("Engine created.");

    return true;
}

bool Engine::recreate(ANativeWindow *window)
{
    if (!created) return false;

    if (outdated)
    {
        vkDeviceWaitIdle(device->get());

        const auto extent = window::getExtent(window);

        delete surface;

        surface = new Surface(instance->get(), window);
        device->updateSurface(surface->get());
        swapChain->recreate(surface->get(), extent);
        mainRenderPass->recreate(extent);
        earthPipeline->recreate();
        scene->resize(extent);

        initGraphicsCommands();

        outdated = false;
    }

    return true;
}

void Engine::outdate()
{
    outdated = true;
}

void Engine::pause()
{
    paused = true;
}

void Engine::unpause()
{
    paused = false;
}

bool Engine::drawFrame()
{
    if (!created || outdated || paused) return false;

    scene->update();

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
        device->get(), 
        swapChain->get(), 
        UINT64_MAX, 
        imageAvailableSemaphore, 
        nullptr, 
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        LOGE("vkAcquireNextImageKHR: VK_ERROR_OUT_OF_DATE_KHR.");
        return false;
    }
    if (result != VK_SUBOPTIMAL_KHR)
    {
        CALL_VK(result);
    }

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
    result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return false;
    }

    CALL_VK(result);

    return true;
}

bool Engine::destroy()
{
    if (!created) return true;

    vkDeviceWaitIdle(device->get());

    for (auto &semaphore : passFinishedSemaphores)
    {
        vkDestroySemaphore(device->get(), semaphore, nullptr);
    }
    vkDestroySemaphore(device->get(), imageAvailableSemaphore, nullptr);

    delete earthPipeline;    
    for (auto descriptor : descriptors)
    {
        delete descriptor;
    }

    delete mainRenderPass;
    delete descriptorPool;
    delete scene;
    delete swapChain;
    delete device;
    delete surface;

    created = false;

    LOGI("Engine destroyed.");

    return true;
}

void Engine::initDescriptorSets()
{
    std::vector<TextureImage*> earthTextures = scene->getEarthTextures();
    const std::vector<VkShaderStageFlags> earthTextureShaderStages(earthTextures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

    descriptors.resize(COUNT);

    descriptors[SCENE] = new DescriptorSets(
        descriptorPool,
        { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
        {});
    descriptors[MODEL] = new DescriptorSets(
        descriptorPool,
        { VK_SHADER_STAGE_VERTEX_BIT },
        earthTextureShaderStages);

    descriptors[SCENE]->pushDescriptorSet({ scene->getCameraBuffer(), scene->getLightingBuffer()  }, {});
    descriptors[MODEL]->pushDescriptorSet({ scene->getEarthTransformationBuffer() }, earthTextures );
}

void Engine::createEarthPipeline()
{
    const std::string shadersPath = "shaders/Earth/";
    const std::vector<std::shared_ptr<ShaderModule>> shaders{
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    earthPipeline = new GraphicsPipeline(
        device,
        mainRenderPass,
        { descriptors[SCENE]->getLayout(), descriptors[MODEL]->getLayout() },
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true);
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

        vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, earthPipeline->get());

        std::vector<VkDescriptorSet> descriptorSets{
            descriptors[SCENE]->getDescriptorSet(0),
            descriptors[MODEL]->getDescriptorSet(0)
        };
        vkCmdBindDescriptorSets(
            graphicsCommands[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            earthPipeline->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);

        scene->drawSphere(graphicsCommands[i]);

        vkCmdEndRenderPass(graphicsCommands[i]);

        CALL_VK(vkEndCommandBuffer(graphicsCommands[i]));
    }

    LOGI("Graphics commands created.");
}
