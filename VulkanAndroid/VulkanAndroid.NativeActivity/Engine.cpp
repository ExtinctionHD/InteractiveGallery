#include "Engine.h"
#include "Vertex.h"
#include "utils.h"
#include "Position.h"

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

    mainRenderPass = new MainRenderPass(device, swapChain->getExtent(), VK_SAMPLE_COUNT_1_BIT);
    toneRenderPass = new ToneRenderPass(device, swapChain);
    mainRenderPass->create();
    toneRenderPass->create();

    descriptorPool = new DescriptorPool(
        device,
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Scene::TEXTURE_COUNT + 1 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Scene::BUFFER_COUNT + 0 }
        },
        DESCRIPTOR_TYPE_COUNT);

    initDescriptorSets();
    initPipelines();

    renderingFinished = createSemaphore();
    imageAvailable = createSemaphore();

    initGraphicsCommands();

    LOGI("Engine created.");

    return created = true;
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
        toneRenderPass->recreate(extent);

        descriptors[DESCRIPTOR_TYPE_TONE]->updateDescriptorSet(
            0,
            { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { mainRenderPass->getTexture() } } });

        for(auto pipeline: pipelines)
        {
            pipeline->recreate();
        }

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
    scene->skipTime();
}

bool Engine::onPause()
{
    return paused;
}

void Engine::handleMotion(glm::vec2 delta)
{
    scene->handleMotion(delta);
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
        imageAvailable, 
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

    std::vector<VkSemaphore> waitSemaphores = {  imageAvailable };
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::vector<VkSemaphore> signalSemaphores = { renderingFinished };
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

    vkDestroySemaphore(device->get(), renderingFinished, nullptr);
    vkDestroySemaphore(device->get(), imageAvailable, nullptr);

    for (auto pipeline : pipelines)
    {
        delete pipeline;
    }

    for (auto descriptor : descriptors)
    {
        delete descriptor;
    }

    delete toneRenderPass;
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

    descriptors.resize(DESCRIPTOR_TYPE_COUNT);

    // Scene:

    descriptors[DESCRIPTOR_TYPE_SCENE] = new DescriptorSets(
        descriptorPool,
        { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT } } });
    descriptors[DESCRIPTOR_TYPE_SCENE]->pushDescriptorSet(
        { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getCameraBuffer(), scene->getLightingBuffer() } } });

    // Earth:

    std::vector<void*> earthTextures;
    for (const auto texture : scene->getEarthTextures())
    {
        earthTextures.push_back(texture);
    }

    descriptors[DESCRIPTOR_TYPE_EARTH] = new DescriptorSets(
        descriptorPool,
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                std::vector<VkShaderStageFlags>(earthTextures.size(), VK_SHADER_STAGE_FRAGMENT_BIT)
            },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { VK_SHADER_STAGE_VERTEX_BIT } },
        });
    descriptors[DESCRIPTOR_TYPE_EARTH]->pushDescriptorSet(
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, earthTextures },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getEarthTransformationBuffer() } }
        });

    // Clouds and skybox:

    descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX] = new DescriptorSets(
        descriptorPool,
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { VK_SHADER_STAGE_FRAGMENT_BIT } },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { VK_SHADER_STAGE_VERTEX_BIT } }
        });
    descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->pushDescriptorSet(
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { scene->getCloudsTexture() } },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getCloudsTransformationBuffer() } }
        });
    descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->pushDescriptorSet(
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { scene->getSkyboxTexture() } },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getSkyboxTransformationBuffer() } }
        });

    // Tone:

    descriptors[DESCRIPTOR_TYPE_TONE] = new DescriptorSets(
        descriptorPool,
        { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { VK_SHADER_STAGE_FRAGMENT_BIT } } });
    descriptors[DESCRIPTOR_TYPE_TONE]->pushDescriptorSet(
        { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { mainRenderPass->getTexture() } } });
}

void Engine::initPipelines()
{
    pipelines.resize(PIPELINE_TYPE_COUNT);

    // Earth:

    std::string shadersPath = "shaders/Earth/";
    std::vector<std::shared_ptr<ShaderModule>> shaders{
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_EARTH] = new GraphicsPipeline(
        device,
        mainRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_EARTH]->getLayout()
        },
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true);

    // Clouds:

    shadersPath = "shaders/Clouds/";
    shaders = {
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_CLOUDS] = new GraphicsPipeline(
        device,
        mainRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getLayout()
        },
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true);

    // Skybox:

    shadersPath = "shaders/Skybox/";
    shaders = {
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_SKYBOX] = new GraphicsPipeline(
        device,
        mainRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getLayout()
        },
        {},
        shaders,
        { Position::getBindingDescription(0) },
        Position::getAttributeDescriptions(0, 0),
        true);

    // Tone:

    shadersPath = "shaders/Tone/";
    shaders = {
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_TONE] = new GraphicsPipeline(
        device,
        toneRenderPass,
        { descriptors[DESCRIPTOR_TYPE_TONE]->getLayout() },
        {},
        shaders,
        {},
        {},
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
        VkRenderPassBeginInfo mainRenderPassBeginInfo{
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            mainRenderPass->get(),
            mainRenderPass->getFramebuffers()[0],
            renderArea,
            uint32_t(clearValues.size()),
            clearValues.data()
        };

        vkCmdBeginRenderPass(graphicsCommands[i], &mainRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Skybox:

        vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_SKYBOX]->get());
        std::vector<VkDescriptorSet> descriptorSets{
            descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getDescriptorSet(1)
        };
        vkCmdBindDescriptorSets(
            graphicsCommands[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_SKYBOX]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        scene->drawCube(graphicsCommands[i]);

        // Earth:

        vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_EARTH]->get());
        descriptorSets = {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
            descriptors[DESCRIPTOR_TYPE_EARTH]->getDescriptorSet(0)
        };
        vkCmdBindDescriptorSets(
            graphicsCommands[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_EARTH]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        scene->drawSphere(graphicsCommands[i]);

        // Clouds:

        vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_CLOUDS]->get());
        descriptorSets = {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getDescriptorSet(0)
        };
        vkCmdBindDescriptorSets(
            graphicsCommands[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_CLOUDS]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        scene->drawSphere(graphicsCommands[i]);

        vkCmdEndRenderPass(graphicsCommands[i]);

        // Tone:

        clearValues = toneRenderPass->getClearValues();
        VkRenderPassBeginInfo toneRenderPassBeginInfo{
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            toneRenderPass->get(),
            toneRenderPass->getFramebuffers()[i],
            renderArea,
            uint32_t(clearValues.size()),
            clearValues.data()
        };

        vkCmdBeginRenderPass(graphicsCommands[i], &toneRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_TONE]->get());
        descriptorSets = {
            descriptors[DESCRIPTOR_TYPE_TONE]->getDescriptorSet(0)
        };
        vkCmdBindDescriptorSets(
            graphicsCommands[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_TONE]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        vkCmdDraw(graphicsCommands[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(graphicsCommands[i]);

        CALL_VK(vkEndCommandBuffer(graphicsCommands[i]));
    }

    LOGI("Graphics commands created.");
}
