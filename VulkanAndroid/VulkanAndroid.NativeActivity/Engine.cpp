#include "Engine.h"

Engine::Engine(ANativeWindow *window)
{
	InitVulkan();

	instance = new Instance();
	surface = new Surface(instance->get(), window);
}

Engine::~Engine()
{
	delete surface;
	delete instance;
}
