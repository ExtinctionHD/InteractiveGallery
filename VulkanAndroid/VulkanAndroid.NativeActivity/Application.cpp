#include "Application.h"
#include "AssetManager.h"
#include "utils.h"

Application::Application(android_app *app) : app(app)
{
    AssetManager::setManager(app->activity->assetManager);

    app->userData = &engine;
    app->onAppCmd = handleAppCommand;
}

void Application::mainLoop()
{
    int events;
    android_poll_source *source;

    while (true)
    {
        while (ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0)
        {
            if (source)
            {
                source->process(app, source);
            }

            if (app->destroyRequested)
            {
                LOGI("Applicatoin destroyed.");
                return;
            }
        }

        engine.drawFrame();
    }
}

void Application::handleAppCommand(android_app *app, int32_t cmd)
{
    LOGA(app->userData, "User data undefined.");
    auto engine = reinterpret_cast<Engine*>(app->userData);

    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
        LOGD("APP_CMD_INIT_WINDOW");
        LOGA(app->window, "Window undefined.");
        if (!engine->recreate(app->window))
        {
            engine->create(app->window);
        }
        break;
    case APP_CMD_TERM_WINDOW:
        LOGD("APP_CMD_TERM_WINDOW");
        engine->outdate();
        break;
    case APP_CMD_GAINED_FOCUS:
        LOGD("APP_CMD_GAINED_FOCUS");
        engine->unpause();
        break;
    case APP_CMD_LOST_FOCUS:
        LOGD("APP_CMD_LOST_FOCUS");
        engine->pause();
        break;
    default: ;
    }
}
