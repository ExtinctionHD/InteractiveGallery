#pragma once
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "MainRenderPass.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"

class Engine
{
public:
    Engine();

	~Engine();

    bool create(ANativeWindow *window);

    bool resize(VkExtent2D newExtent);

    bool drawFrame();

    bool destroy();

private:
	Instance *instance;

	Surface *surface;

	Device *device;

    SwapChain *swapChain;

    MainRenderPass *mainRenderPass;

    DescriptorPool *descriptorPool;

    GraphicsPipeline *graphicsPipeline;

    VkSemaphore imageAvailableSemaphore;

    std::vector<VkSemaphore> passFinishedSemaphores;

    std::vector<VkCommandBuffer> graphicsCommands;

    bool created;

    VkSemaphore createSemaphore() const;

    void initGraphicsCommands();
};

