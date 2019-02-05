#pragma once
#include "Instance.h"
#include "Surface.h"

class Engine
{
public:
	Engine(ANativeWindow *window);

	~Engine();


private:
	Instance *instance{};

	Surface *surface{};
};

