#include "Engine.h"
#include "Vertex.h"
#include "utils.h"
#include "Position.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "PositionUv.h"

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

    earthRenderPass = new EarthRenderPass(device, swapChain->getExtent(), VK_SAMPLE_COUNT_1_BIT);
    galleryRenderPass = new GalleryRenderPass(device, swapChain, VK_SAMPLE_COUNT_1_BIT);
    earthRenderPass->create();
    galleryRenderPass->create();

    descriptorPool = new DescriptorPool(
        device,
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Scene::TEXTURE_COUNT + 2 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swapChain->getImageCount() + 1 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Scene::BUFFER_COUNT + 0 }
        },
        DESCRIPTOR_TYPE_COUNT + 1 + swapChain->getImageCount());

    createLuminosityImage();

    initDescriptorSets();
    initLocalGroupSize();
    initPipelines();

    earthRenderingFinished = createSemaphore();
    computingFinished = createSemaphore();
    galleryRenderingFinished = createSemaphore();
    imageAvailable = createSemaphore();

    initEarthRenderingCommands(); 
    initComputingCommands();
    initGalleryRenderingCommands();

    LOGI("Engine created.");

    return created = true;
}

bool Engine::recreate(ANativeWindow *window)
{
    if (!created) return false;

    if (outdated)
    {
        vkDeviceWaitIdle(device->get());

        delete surface;
        surface = new Surface(instance->get(), window);
        device->updateSurface(surface->get());

        const auto extent = window::getExtent(window);
        swapChain->recreate(surface->get(), extent);
        earthRenderPass->recreate(extent);
        galleryRenderPass->recreate(extent);
        scene->resize(extent);
        for (auto pipeline : pipelines)
        {
            pipeline->recreate();
        }

        updateChangedDescriptorSets();

        initEarthRenderingCommands();
        initComputingCommands();
        initGalleryRenderingCommands();

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

void Engine::handleZoom(float delta)
{
    scene->handleZoom(delta);
}

void Engine::activateGallery()
{
    scene->activateGallery();
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

    // Earth rendering:

    std::vector<VkSemaphore> earthRenderingSignalSemaphores{ earthRenderingFinished };
    VkSubmitInfo renderingSubmitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        1,
        &earthRenderingCommands,
        uint32_t(earthRenderingSignalSemaphores.size()),
        earthRenderingSignalSemaphores.data(),
    };
    CALL_VK(vkQueueSubmit(device->getGraphicsQueue(), 1, &renderingSubmitInfo, nullptr));

    // Computing:

    std::vector<VkSemaphore> computingWaitSemaphores{ earthRenderingFinished, imageAvailable};
    std::vector<VkPipelineStageFlags> computingWaitStages{ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
    std::vector<VkSemaphore> computingSignalSemaphores{ computingFinished };
    VkSubmitInfo computingSubmitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        uint32_t(computingWaitSemaphores.size()),
        computingWaitSemaphores.data(),
        computingWaitStages.data(),
        1,
        &computingCommands[imageIndex],
        uint32_t(computingSignalSemaphores.size()),
        computingSignalSemaphores.data(),
    };
    CALL_VK(vkQueueSubmit(device->getComputeQueue(), 1, &computingSubmitInfo, nullptr));

    // Gallery rendering:

    std::vector<VkSemaphore> galleryRenderingWaitSemaphores{ computingFinished };
    std::vector<VkPipelineStageFlags> galleryWaitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::vector<VkSemaphore> gallerySignalSemaphores{ galleryRenderingFinished };
    VkSubmitInfo gallerySubmitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        uint32_t(galleryRenderingWaitSemaphores.size()),
        galleryRenderingWaitSemaphores.data(),
        galleryWaitStages.data(),
        1,
        &galleryRenderingCommands[imageIndex],
        uint32_t(gallerySignalSemaphores.size()),
        gallerySignalSemaphores.data(),
    };
    CALL_VK(vkQueueSubmit(device->getComputeQueue(), 1, &gallerySubmitInfo, nullptr));

    std::vector<VkSwapchainKHR> swapChains{ swapChain->get() };
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        uint32_t(gallerySignalSemaphores.size()),
        gallerySignalSemaphores.data(),
        uint32_t(swapChains.size()),
        swapChains.data(),
        &imageIndex,
        nullptr,
    };
    result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        LOGI("vkQueuePresentKHR: VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR");
        return false;
    }

    CALL_VK(result);

    return true;
}

bool Engine::destroy()
{
    if (!created) return true;

    vkDeviceWaitIdle(device->get());

    vkDestroySemaphore(device->get(), imageAvailable, nullptr);
    vkDestroySemaphore(device->get(), galleryRenderingFinished, nullptr);
    vkDestroySemaphore(device->get(), computingFinished, nullptr);
    vkDestroySemaphore(device->get(), earthRenderingFinished, nullptr);

    for (auto pipeline : pipelines)
    {
        delete pipeline;
    }

    for (auto descriptor : descriptors)
    {
        delete descriptor;
    }

    delete galleryRenderPass;
    delete earthRenderPass;
    delete descriptorPool;
    delete luminosityImage;
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
        {
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                { scene->getCameraBuffer()->getUniformBufferInfo(), scene->getLightingBufferInfo() }
            }
        });

    // Earth:

    std::vector<DescriptorInfo> earthTextureInfos = scene->getModelTextureInfos(Scene::ModelId::EARTH);
    descriptors[DESCRIPTOR_TYPE_EARTH] = new DescriptorSets(
        descriptorPool,
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                std::vector<VkShaderStageFlags>(earthTextureInfos.size(), VK_SHADER_STAGE_FRAGMENT_BIT)
            },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { VK_SHADER_STAGE_VERTEX_BIT } },
        });
    descriptors[DESCRIPTOR_TYPE_EARTH]->pushDescriptorSet(
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, earthTextureInfos },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getModelTransformationBufferInfo(Scene::ModelId::EARTH) } }
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
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, scene->getModelTextureInfos(Scene::ModelId::CLOUDS) },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getModelTransformationBufferInfo(Scene::ModelId::CLOUDS) } }
        });
    descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->pushDescriptorSet(
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, scene->getModelTextureInfos(Scene::ModelId::SKYBOX) },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { scene->getModelTransformationBufferInfo(Scene::ModelId::SKYBOX) } }
        });

    // Luminosity:

    const auto colorTexture = earthRenderPass->getColorTexture();

    descriptors[DESCRIPTOR_TYPE_LUMINOSITY_SRC] = new DescriptorSets(
        descriptorPool,
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                { VK_SHADER_STAGE_COMPUTE_BIT }
            }
        });
    descriptors[DESCRIPTOR_TYPE_LUMINOSITY_SRC]->pushDescriptorSet(
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                { colorTexture->getCombineSamplerInfo(0, 1) }
            }
        });

    descriptors[DESCRIPTOR_TYPE_LUMINOSITY_DST] = new DescriptorSets(
        descriptorPool,
        {
            {
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                { VK_SHADER_STAGE_COMPUTE_BIT }
            }
        });
    descriptors[DESCRIPTOR_TYPE_LUMINOSITY_DST]->pushDescriptorSet(
        {
            {
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                { luminosityImage->getStorageImageInfo() }
            }
        });

    // Tone:

    descriptors[DESCRIPTOR_TYPE_TONE_SRC] = new DescriptorSets(
        descriptorPool,
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                { VK_SHADER_STAGE_COMPUTE_BIT }
            }
        });
    descriptors[DESCRIPTOR_TYPE_TONE_SRC]->pushDescriptorSet(
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                { colorTexture->getCombineSamplerInfo() }
            }
        });

    descriptors[DESCRIPTOR_TYPE_TONE_DST] = new DescriptorSets(
        descriptorPool,
        { { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { VK_SHADER_STAGE_COMPUTE_BIT } } });
    for (const auto swapChainImage : swapChain->getImages())
    {
        descriptors[DESCRIPTOR_TYPE_TONE_DST]->pushDescriptorSet(
            { { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { swapChainImage->getStorageImageInfo() } } });
    }

    // Gallery:

    descriptors[DESCRIPTOR_TYPE_GALLERY] = new DescriptorSets(
        descriptorPool,
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { VK_SHADER_STAGE_FRAGMENT_BIT } },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT } }
        });
    descriptors[DESCRIPTOR_TYPE_GALLERY]->pushDescriptorSet(
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, scene->getModelTextureInfos(Scene::ModelId::GALLERY) },
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                {
                    scene->getModelTransformationBufferInfo(Scene::ModelId::GALLERY),
                    scene->getModelUniformBufferInfo(Scene::ModelId::GALLERY)[0]
                }
            }
        });
}

void Engine::initLocalGroupSize()
{
    const auto extent = swapChain->getExtent();

    while (extent.width % localGroupSize.x != 0)
    {
        localGroupSize.x /= 2;
    }

    while (extent.height % localGroupSize.y != 0)
    {
        localGroupSize.y /= 2;
    }

    LOGD("Local group size: %d x %d", localGroupSize.x, localGroupSize.y);
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
        earthRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_EARTH]->getLayout()
        },
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true,
        false);

    // Clouds:

    shadersPath = "shaders/Clouds/";
    shaders = {
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_CLOUDS] = new GraphicsPipeline(
        device,
        earthRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getLayout()
        },
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true,
        true);

    // Skybox:

    shadersPath = "shaders/Skybox/";
    shaders = {
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_SKYBOX] = new GraphicsPipeline(
        device,
        earthRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getLayout()
        },
        {},
        shaders,
        { Position::getBindingDescription(0) },
        Position::getAttributeDescriptions(0, 0),
        true,
        false);

    // Luminosity:

    const std::shared_ptr<ShaderModule> luminosityShader = std::make_shared<ShaderModule>(
        device,
        "shaders/Luminosity/comp.spv",
        VK_SHADER_STAGE_COMPUTE_BIT);

    pipelines[PIPELINE_TYPE_LUMINOSITY] = new ComputePipeline(
        device,
        {
            descriptors[DESCRIPTOR_TYPE_LUMINOSITY_SRC]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_LUMINOSITY_DST]->getLayout()
        },
        {},
        luminosityShader);

    // Tone:

    std::vector<VkSpecializationMapEntry> specializationEntries{
        {0, 0, sizeof localGroupSize.x},
        {1, sizeof localGroupSize.x, sizeof localGroupSize.y}
    };

    std::vector<const void*> data{
        &localGroupSize.x,
        &localGroupSize.y
    };

    const std::shared_ptr<ShaderModule> toneShader = std::make_shared<ShaderModule>(
        device,
        "shaders/Tone/comp.spv",
        VK_SHADER_STAGE_COMPUTE_BIT,
        specializationEntries,
        data);

    pipelines[PIPELINE_TYPE_TONE] = new ComputePipeline(
        device,
        {
            descriptors[DESCRIPTOR_TYPE_LUMINOSITY_DST]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_TONE_SRC]->getLayout(), 
            descriptors[DESCRIPTOR_TYPE_TONE_DST]->getLayout()
        },
        {},
        toneShader);

    // Gallery:

    shadersPath = "shaders/Gallery/";
    shaders = {
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelines[PIPELINE_TYPE_GALLERY] = new GraphicsPipeline(
        device,
        galleryRenderPass,
        {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getLayout(),
            descriptors[DESCRIPTOR_TYPE_GALLERY]->getLayout()
        },
        {},
        shaders,
        { PositionUv::getBindingDescription(0) },
        PositionUv::getAttributeDescriptions(0, 0),
        false,
        true);
}

void Engine::createLuminosityImage()
{
    luminosityImage = new Image(
        device,
        0,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        { 1, 1, 1 },
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT,
        false);
    luminosityImage->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);

    using namespace glm::detail;

    const std::vector<hdata> defaultLuminosity{
        toFloat16(1.0f),
        toFloat16(1.0f),
        toFloat16(1.0f),
        toFloat16(1.0f),
    };

    luminosityImage->updateData({ defaultLuminosity.data() }, 0, defaultLuminosity.size() * sizeof(hdata));
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

void Engine::initEarthRenderingCommands()
{
    const VkCommandPool commandPool = device->getCommandPool();

    if (earthRenderingCommands)
    {
        vkFreeCommandBuffers(device->get(), commandPool, 1, &earthRenderingCommands);
    }

    VkCommandBufferAllocateInfo allocInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };

    CALL_VK(vkAllocateCommandBuffers(device->get(), &allocInfo, &earthRenderingCommands));

    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        nullptr,
    };

    CALL_VK(vkBeginCommandBuffer(earthRenderingCommands, &beginInfo));

    {
        const VkRect2D renderArea{
        { 0, 0 },
        earthRenderPass->getExtent()
        };
        auto clearValues = earthRenderPass->getClearValues();
        VkRenderPassBeginInfo mainRenderPassBeginInfo{
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            earthRenderPass->get(),
            earthRenderPass->getFramebuffers().front(),
            renderArea,
            uint32_t(clearValues.size()),
            clearValues.data()
        };

        vkCmdBeginRenderPass(earthRenderingCommands, &mainRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Skybox:

        vkCmdBindPipeline(earthRenderingCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_SKYBOX]->get());
        std::vector<VkDescriptorSet> descriptorSets{
            descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getDescriptorSet(1)
        };
        vkCmdBindDescriptorSets(
            earthRenderingCommands,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_SKYBOX]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        scene->drawCube(earthRenderingCommands);

        // Earth:

        vkCmdBindPipeline(earthRenderingCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_EARTH]->get());
        descriptorSets = {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
            descriptors[DESCRIPTOR_TYPE_EARTH]->getDescriptorSet(0)
        };
        vkCmdBindDescriptorSets(
            earthRenderingCommands,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_EARTH]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        scene->drawSphere(earthRenderingCommands);

        // Clouds:

        vkCmdBindPipeline(earthRenderingCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_CLOUDS]->get());
        descriptorSets = {
            descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
            descriptors[DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX]->getDescriptorSet(0)
        };
        vkCmdBindDescriptorSets(
            earthRenderingCommands,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelines[PIPELINE_TYPE_CLOUDS]->getLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr);
        scene->drawSphere(earthRenderingCommands);

        vkCmdEndRenderPass(earthRenderingCommands);
    }

    CALL_VK(vkEndCommandBuffer(earthRenderingCommands));

    LOGI("Earth rendering commands initialized.");
}

void Engine::initComputingCommands()
{
    const VkCommandPool commandPool = device->getCommandPool();
    const uint32_t count = swapChain->getImageCount();

    if (!computingCommands.empty())
    {
        vkFreeCommandBuffers(device->get(), commandPool, uint32_t(computingCommands.size()), computingCommands.data());
    }

    computingCommands.resize(count);

    VkCommandBufferAllocateInfo allocInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        count,
    };

    CALL_VK(vkAllocateCommandBuffers(device->get(), &allocInfo, computingCommands.data()));

    for (uint32_t i = 0; i < count; i++)
    {

        VkCommandBufferBeginInfo beginInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            nullptr,
        };

        CALL_VK(vkBeginCommandBuffer(computingCommands[i], &beginInfo));

        {
            const auto colorTexture = earthRenderPass->getColorTexture();

            colorTexture->memoryBarrier(
                computingCommands[i],
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT, 
                    0,
                    1, 
                    0, 
                    1
                });
            colorTexture->memoryBarrier(
                computingCommands[i],
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_ACCESS_SHADER_READ_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    1,
                    colorTexture->getMipLevelCount() - 1,
                    0,
                    1
                });

            colorTexture->generateMipmaps(
                computingCommands[i],
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_FILTER_LINEAR,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            vkCmdBindPipeline(computingCommands[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipelines[PIPELINE_TYPE_LUMINOSITY]->get());
            std::vector<VkDescriptorSet> lumDescriptorSets{
                descriptors[DESCRIPTOR_TYPE_LUMINOSITY_SRC]->getDescriptorSet(0),
                descriptors[DESCRIPTOR_TYPE_LUMINOSITY_DST]->getDescriptorSet(0)
            };
            vkCmdBindDescriptorSets(
                computingCommands[i],
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipelines[PIPELINE_TYPE_LUMINOSITY]->getLayout(),
                0,
                lumDescriptorSets.size(),
                lumDescriptorSets.data(),
                0,
                nullptr);
            vkCmdDispatch(computingCommands[i], 1, 1, 1);

            const auto swapChainImage = swapChain->getImages()[i];
            swapChainImage->memoryBarrier(
                computingCommands[i],
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_GENERAL,
                0,
                VK_ACCESS_SHADER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    1,
                    0,
                    1
                });

            vkCmdBindPipeline(computingCommands[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipelines[PIPELINE_TYPE_TONE]->get());
            std::vector<VkDescriptorSet> toneDescriptorSets{
                descriptors[DESCRIPTOR_TYPE_LUMINOSITY_DST]->getDescriptorSet(0),
                descriptors[DESCRIPTOR_TYPE_TONE_SRC]->getDescriptorSet(0),
                descriptors[DESCRIPTOR_TYPE_TONE_DST]->getDescriptorSet(i)
            };
            vkCmdBindDescriptorSets(
                computingCommands[i],
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipelines[PIPELINE_TYPE_TONE]->getLayout(),
                0,
                toneDescriptorSets.size(),
                toneDescriptorSets.data(),
                0,
                nullptr);

            const auto imageExtent = swapChain->getExtent();
            vkCmdDispatch(computingCommands[i], imageExtent.width / localGroupSize.x, imageExtent.height / localGroupSize.y, 1);

            colorTexture->memoryBarrier(
                computingCommands[i],
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_SHADER_READ_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    1,
                    0,
                    1
                });

            swapChainImage->memoryBarrier(
                computingCommands[i],
                VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_SHADER_WRITE_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    1,
                    0,
                    1
                });
        }

        CALL_VK(vkEndCommandBuffer(computingCommands[i]));
    }

    LOGI("Computing commands initialized.");
}

void Engine::initGalleryRenderingCommands()
{
    const VkCommandPool commandPool = device->getCommandPool();
    const uint32_t count = swapChain->getImageCount();

    if (!galleryRenderingCommands.empty())
    {
        vkFreeCommandBuffers(device->get(), commandPool, uint32_t(galleryRenderingCommands.size()), galleryRenderingCommands.data());
    }

    galleryRenderingCommands.resize(count);

    VkCommandBufferAllocateInfo allocInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        count,
    };

    CALL_VK(vkAllocateCommandBuffers(device->get(), &allocInfo, galleryRenderingCommands.data()));

    for (uint32_t i = 0; i < count; i++)
    {

        VkCommandBufferBeginInfo beginInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            nullptr,
        };

        CALL_VK(vkBeginCommandBuffer(galleryRenderingCommands[i], &beginInfo));

        {
            const VkRect2D renderArea{
            { 0, 0 },
            galleryRenderPass->getExtent()
            };
            auto clearValues = galleryRenderPass->getClearValues();
            VkRenderPassBeginInfo galleryRenderPassBeginInfo{
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                nullptr,
                galleryRenderPass->get(),
                galleryRenderPass->getFramebuffers()[i],
                renderArea,
                uint32_t(clearValues.size()),
                clearValues.data()
            };

            vkCmdBeginRenderPass(galleryRenderingCommands[i], &galleryRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
            vkCmdBindPipeline(galleryRenderingCommands[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PIPELINE_TYPE_GALLERY]->get());
            std::vector<VkDescriptorSet> descriptorSets{
                descriptors[DESCRIPTOR_TYPE_SCENE]->getDescriptorSet(0),
                descriptors[DESCRIPTOR_TYPE_GALLERY]->getDescriptorSet(0),
            };
            vkCmdBindDescriptorSets(
                galleryRenderingCommands[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelines[PIPELINE_TYPE_GALLERY]->getLayout(),
                0,
                descriptorSets.size(),
                descriptorSets.data(),
                0,
                nullptr);

            scene->drawCard(galleryRenderingCommands[i]);

            vkCmdEndRenderPass(galleryRenderingCommands[i]);
        }

        CALL_VK(vkEndCommandBuffer(galleryRenderingCommands[i]));
    }

    LOGI("Gallery rendering commands initialized.");
}

void Engine::updateChangedDescriptorSets()
{
    const auto colorTexture = earthRenderPass->getColorTexture();

    descriptors[DESCRIPTOR_TYPE_LUMINOSITY_SRC]->updateDescriptorSet(
        0,
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                { colorTexture->getCombineSamplerInfo(0, 1) }
            }
        });

    descriptors[DESCRIPTOR_TYPE_TONE_SRC]->updateDescriptorSet(
        0,
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                { colorTexture->getCombineSamplerInfo() }
            }
        });
    const auto swapChainImages = swapChain->getImages();
    for (uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        descriptors[DESCRIPTOR_TYPE_TONE_DST]->updateDescriptorSet(
            i,
            { { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, { swapChainImages[i]->getStorageImageInfo() } } });
    }
}
