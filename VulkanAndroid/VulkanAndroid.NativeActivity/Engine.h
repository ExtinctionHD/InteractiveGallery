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
        SCENE,
        MODEL,
        COUNT
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

    DescriptorPool *descriptorPool;

    std::vector<DescriptorSets*> descriptors;

    GraphicsPipeline *earthPipeline;

    VkSemaphore imageAvailableSemaphore;

    std::vector<VkSemaphore> passFinishedSemaphores;

    std::vector<VkCommandBuffer> graphicsCommands;

    void initDescriptorSets();

    void createEarthPipeline();

    VkSemaphore createSemaphore() const;

    void initGraphicsCommands();
};

