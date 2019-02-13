#include "android_native_app_glue.h"
#include "Engine.h"
#include "AssetManager.h"

void android_main(android_app *app) 
{
	LOGI("Application started.");

    int events;

    android_poll_source *source;
    Engine *engine = nullptr;

    AssetManager::setManager(app->activity->assetManager);

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
                LOGI("Applicatoin destroy requested.");
                delete engine;
                return;
            }
        }

        if (app->window && !engine)
        {
            engine = new Engine(app->window);
        }

        if (engine) 
        {
            engine->drawFrame();
        }
    }
}
