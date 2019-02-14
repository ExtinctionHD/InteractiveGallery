#include "android_native_app_glue.h"
#include "Engine.h"
#include "AssetManager.h"
#include "general.h"

bool focused = false;

void handleAppCommand(android_app *app, int32_t cmd)
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
    case APP_CMD_WINDOW_RESIZED:
        LOGD("APP_CMD_WINDOW_RESIZED");
        engine->resize(window::getExtent(app->window));
        break;
    case APP_CMD_GAINED_FOCUS:
        LOGD("APP_CMD_GAINED_FOCUS");
        focused = true;
        break;
    case APP_CMD_LOST_FOCUS:
        LOGD("APP_CMD_LOST_FOCUS");
        focused = false;
        break;
    default: ;
    }
}

void android_main(android_app *app) 
{
	LOGI("Application started.");

    int events;
    android_poll_source *source;
    Engine engine;

    AssetManager::setManager(app->activity->assetManager);

    app->userData = &engine;
    app->onAppCmd = handleAppCommand;

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

        if (focused) 
        {
            engine.drawFrame();
        }
    }
}
