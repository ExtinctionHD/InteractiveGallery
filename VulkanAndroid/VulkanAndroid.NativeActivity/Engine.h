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

    bool onPause();

    void handleMotion(glm::vec2 delta);

    bool drawFrame();

    bool destroy();

private:
    enum DescriptorType
    {
        DESCRIPTOR_TYPE_SCENE,
        DESCRIPTOR_TYPE_EARTH,
        DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX = 3,
        DESCRIPTOR_TYPE_TONE,
        DESCRIPTOR_TYPE_COUNT
    };

    enum PipelineType
    {
        PIPELINE_TYPE_EARTH,
        PIPELINE_TYPE_CLOUDS,
        PIPELINE_TYPE_SKYBOX,
        PIPELINE_TYPE_TONE,
        PIPELINE_TYPE_COUNT
    };

    bool created;

    bool outdated;

    bool paused;

	Instance *instance;

	Surface *surface;

	Device *device;

    SwapChain *swapChain;

    DescriptorPool *descriptorPool;

    MainRenderPass *mainRenderPass;

    ToneRenderPass *toneRenderPass;

    std::vector<DescriptorSets*> descriptors;

    std::vector<GraphicsPipeline*> pipelines;

    Scene *scene;

    VkSemaphore renderingFinished;

    VkSemaphore imageAvailable;

    std::vector<VkCommandBuffer> graphicsCommands;

    void initDescriptorSets();

    void initPipelines();

    VkSemaphore createSemaphore() const;

    void initGraphicsCommands();
};

