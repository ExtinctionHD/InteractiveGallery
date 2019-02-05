#include "android_native_app_glue.h"
#include "Engine.h"

void android_main(android_app *app) 
{
	LOGI("Application started");

	// Engine engine(app->window);

	int events;
	android_poll_source *source = nullptr;
	Engine *engine = nullptr;

	while (true)
	{
		while (ALooper_pollAll(-1, nullptr, &events, reinterpret_cast<void**>(&source)))
		{
            if (app->window && !engine)
            {
				engine = new Engine(app->window);
            }

			if (source)
			{
				source->process(app, source);
			}

			if (app->destroyRequested)
			{
				LOGI("Application destroyed");

				delete engine;
				return;
			}
		}
	}
}
