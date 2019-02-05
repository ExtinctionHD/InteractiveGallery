#pragma once
#include "Instance.h"
#include "Surface.h"
#include "Device.h"

class Engine
{
public:
	Engine(ANativeWindow *window);

	~Engine();


private:
	Instance *instance;

	Surface *surface;

	Device *device;
};

