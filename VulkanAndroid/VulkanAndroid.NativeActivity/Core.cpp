#include "Core.h"

Core::Core()
{
	InitVulkan();

	instance = new Instance();
}

Core::~Core()
{
	delete instance;
}
