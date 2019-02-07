#include "Engine.h"

Engine::Engine(ANativeWindow *window)
{
	InitVulkan();

	instance = new Instance();
	surface = new Surface(instance->get(), window);
	device = new Device(instance->get(), surface->get(), instance->getLayers());
}

Engine::~Engine()
{
	delete device;
	delete surface;
	delete instance;
}