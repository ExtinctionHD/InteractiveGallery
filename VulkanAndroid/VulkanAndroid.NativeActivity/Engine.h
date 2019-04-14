#pragma once
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "EarthRenderPass.h"
#include "DescriptorPool.h"
#include "Pipeline.h"
#include "Scene.h"
#include "DescriptorSets.h"
#include "GalleryRenderPass.h"

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

    void handleZoom(float delta);

    bool drawFrame();

    bool destroy();

private:
    enum DescriptorType
    {
        DESCRIPTOR_TYPE_SCENE,
        DESCRIPTOR_TYPE_EARTH,
        DESCRIPTOR_TYPE_CLOUDS_AND_SKYBOX,
        DESCRIPTOR_TYPE_TONE_SRC,
        DESCRIPTOR_TYPE_TONE_DST,
        DESCRIPTOR_TYPE_GALLERY,
        DESCRIPTOR_TYPE_COUNT
    };

    enum PipelineType
    {
        PIPELINE_TYPE_EARTH,
        PIPELINE_TYPE_CLOUDS,
        PIPELINE_TYPE_SKYBOX,
        PIPELINE_TYPE_TONE,
        PIPELINE_TYPE_GALLERY,
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

    EarthRenderPass *earthRenderPass;

    GalleryRenderPass *galleryRenderPass;

    std::vector<DescriptorSets*> descriptors;

    std::vector<Pipeline*> pipelines;

    Scene *scene;

    // TODO: combine semaphores

    VkSemaphore earthRenderingFinished;

    VkSemaphore computingFinished;

    VkSemaphore galleryRenderingFinished;

    VkSemaphore imageAvailable;

    // TODO: combine commands

    VkCommandBuffer earthRenderingCommands{};

    glm::uvec2 localGroupSize{ 16 };

    std::vector<VkCommandBuffer> computingCommands{};

    std::vector<VkCommandBuffer> galleryRenderingCommands{};

    void initDescriptorSets();

    void initLocalGroupSize();

    void initPipelines();

    VkSemaphore createSemaphore() const;

    void initEarthRenderingCommands();

    void initComputingCommands();

    void initGalleryRenderingCommands();

    void updateChangedDescriptorSets();
};

