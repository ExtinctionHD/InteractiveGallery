#include "Engine.h"
#include "Vertex.h"
#include "general.h"
#include "sphere.h"

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

    const auto extent = window::getExtent(window);
    LOGD("Window extent: %d x %d.", extent.width, extent.height);

    surface = new Surface(instance->get(), window);
    device = new Device(instance->get(), surface->get(), instance->getLayers());
    swapChain = new SwapChain(device, surface->get(), extent);
    descriptorPool = new DescriptorPool(device, 1, 0, 1);
    mainRenderPass = new MainRenderPass(device, swapChain);
    mainRenderPass->create();

    const Camera::Attributes attributes{
        extent,
        glm::vec3(0.0f, 0.0f, -500.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        60.0f,
        1.0f,
        800.0f
    };
    camera = new Camera(device, attributes);

    descriptor.layout = descriptorPool->createDescriptorSetLayout({ VK_SHADER_STAGE_VERTEX_BIT }, {});
    descriptor.sets = { descriptorPool->getDescriptorSet(descriptor.layout) };
    descriptorPool->updateDescriptorSet(descriptor.sets[0], { camera->getBuffer() }, {});

    const std::string shadersPath = "shaders/Main/";
    const std::vector<std::shared_ptr<ShaderModule>> shaders{
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    graphicsPipeline = new GraphicsPipeline(
        device,
        mainRenderPass,
        { descriptor.layout },
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true);

    createMesh();

    imageAvailableSemaphore = createSemaphore();
    passFinishedSemaphores.resize(RenderPass::LAST + 1, createSemaphore());
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
        graphicsPipeline->recreate();

        initGraphicsCommands();

        outdated = false;
    }

    return true;
}

bool Engine::resize(VkExtent2D newExtent)
{
    if (!created) return false;

    vkDeviceWaitIdle(device->get());

    swapChain->recreate(newExtent);
    mainRenderPass->recreate(newExtent);
    graphicsPipeline->recreate();

    initGraphicsCommands();

    LOGI("Engine resized.");

    return true;
}

void Engine::outdate()
{
    outdated = true;
}

bool Engine::drawFrame()
{
    if (!created || outdated) return false;

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

    vkDestroyDescriptorSetLayout(device->get(), descriptor.layout, nullptr);

    delete indexBuffer;
    delete vertexBuffer;
    delete graphicsPipeline;
    delete camera;
    delete mainRenderPass;
    delete descriptorPool;
    delete swapChain;
    delete device;
    delete surface;

    created = false;

    LOGI("Engine destroyed.");

    return true;
}

void Engine::createMesh()
{
    vertexBuffer = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sphere::VERTICES.size() * sizeof(Vertex));
    vertexBuffer->updateData(sphere::VERTICES.data());

    indexBuffer = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sphere::INDICES.size() * sizeof(uint32_t));
    indexBuffer->updateData(sphere::INDICES.data());

    indexCount = sphere::INDICES.size();

    LOGI("Mesh created.");
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

        vkCmdBindPipeline(graphicsCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->get());

        vkCmdBindDescriptorSets(
            graphicsCommands[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipeline->getLayout(),
            0,
            descriptor.sets.size(),
            descriptor.sets.data(),
            0,
            nullptr);

        VkBuffer buffer = vertexBuffer->get();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(graphicsCommands[i], 0, 1, &buffer, &offset);

        buffer = indexBuffer->get();
        vkCmdBindIndexBuffer(graphicsCommands[i], buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(graphicsCommands[i], indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(graphicsCommands[i]);

        CALL_VK(vkEndCommandBuffer(graphicsCommands[i]));
    }

    LOGI("Graphics commands created.");
}
