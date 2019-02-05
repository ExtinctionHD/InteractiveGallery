#include "Engine.h"

Engine::Engine()
{
	InitVulkan();

	instance = new Instance();
}

Engine::~Engine()
{
	delete instance;
}
