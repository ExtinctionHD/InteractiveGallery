#pragma once
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "MainRenderPass.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "Camera.h"

class Engine
{
public:
    Engine();

	~Engine();

    bool create(ANativeWindow *window);

    bool recreate(ANativeWindow *window);

    bool resize(VkExtent2D newExtent);

    void outdate();

    bool drawFrame();

    bool destroy();

private:
    bool created;

    bool outdated;

	Instance *instance;

	Surface *surface;

	Device *device;

    SwapChain *swapChain;

    MainRenderPass *mainRenderPass;

    DescriptorPool *descriptorPool;

    GraphicsPipeline *graphicsPipeline;

    Buffer *vertexBuffer;

    Buffer *indexBuffer;

    uint32_t indexCount;

    Camera *camera;

    DescriptorStruct descriptor;

    VkSemaphore imageAvailableSemaphore;

    std::vector<VkSemaphore> passFinishedSemaphores;

    std::vector<VkCommandBuffer> graphicsCommands;

    void createMesh();

    VkSemaphore createSemaphore() const;

    void initGraphicsCommands();
};

