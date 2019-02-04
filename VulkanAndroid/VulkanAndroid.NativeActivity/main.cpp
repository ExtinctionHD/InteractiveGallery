#include "android_native_app_glue.h"
#include "Core.h"

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(android_app *app) 
{
	int events;
	android_poll_source *source = nullptr;
	Core core;

	LOGI("Android main entry");

	while (true)
	{
		while (ALooper_pollAll(-1, nullptr, &events, reinterpret_cast<void**>(&source)))
		{
			if (source)
			{
				source->process(app, source);
			}

			if (app->destroyRequested)
			{
				LOGI("Destroy activity");
				return;
			}
		}
	}
}
