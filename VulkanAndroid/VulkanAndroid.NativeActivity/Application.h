#pragma once
#include "android_native_app_glue.h"
#include "Engine.h"

class Application
{
public:
    Application(android_app *app);

    ~Application() = default;

    void mainLoop();

private:
    android_app *app;

    Engine engine;

    static void handleAppCommand(android_app *app, int32_t cmd);

    static bool equals(VkExtent2D x, VkExtent2D y);
};

