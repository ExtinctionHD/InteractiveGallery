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
	Engine(ANativeWindow *window);

	~Engine();

private:
	Instance *instance;

	Surface *surface;

	Device *device;

    SwapChain *swapChain;

    MainRenderPass *mainRenderPass;

    DescriptorPool *descriptorPool;

    GraphicsPipeline *graphicsPipeline;
};

