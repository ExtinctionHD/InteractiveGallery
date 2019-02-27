#pragma once
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "MainRenderPass.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "Scene.h"
#include "DescriptorSets.h"
#include "ToneRenderPass.h"

class Engine
{
public:
    Engine();

	~Engine();

    bool create(ANativeWindow *window);

    bool recreate(ANativeWindow *window);

    void outdate();

    void pause();

    void unpause();

    void handleMotion(glm::vec2 delta);

    bool drawFrame();

    bool destroy();

private:
    enum DescriptorType
    {
        DESCRIPTOR_TYPE_SCENE,
        DESCRIPTOR_TYPE_EARTH,
        DESCRIPTOR_TYPE_CLOUDS,
        DESCRIPTOR_TYPE_SKYBOX,
        DESCRIPTOR_TYPE_TONE,
        DESCRIPTOR_TYPE_COUNT
    };

    bool created;

    bool outdated;

    bool paused;

	Instance *instance;

	Surface *surface;

	Device *device;

    SwapChain *swapChain;

    Scene *scene;

    MainRenderPass *mainRenderPass;

    ToneRenderPass *toneRenderPass;

    DescriptorPool *descriptorPool;

    std::vector<DescriptorSets*> descriptors;

    // TODO combine pipelines in vector

    GraphicsPipeline *earthPipeline;

    GraphicsPipeline *cloudsPipeline;

    GraphicsPipeline *skyboxPipeline;

    GraphicsPipeline *tonePipeline;

    VkSemaphore imageAvailable;

    VkSemaphore renderingFinished;

    std::vector<VkCommandBuffer> graphicsCommands;

    void initDescriptorSets();

    void createEarthPipeline();

    void createCloudsPipeline();

    void createSkyboxPipeline();

    void createTonePipeline();

    VkSemaphore createSemaphore() const;

    void initGraphicsCommands();
};

