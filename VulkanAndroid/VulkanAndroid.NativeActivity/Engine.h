#pragma once
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "MainRenderPass.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include <map>

class Engine
{
public:
	Engine(ANativeWindow *window);

	~Engine();

    void drawFrame();

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

    VkSemaphore createSemaphore() const;

    void initGraphicsCommands();
};

