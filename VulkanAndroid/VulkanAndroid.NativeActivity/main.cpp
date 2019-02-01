#include "android_native_app_glue.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "VulkanAndroid", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "VulkanAndroid", __VA_ARGS__))

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(android_app *app) 
{
	int events;
	android_poll_source *source = nullptr;

	LOGI("Android main entry");

	while (true)
	{
		while (ALooper_pollAll(-1, nullptr, &events, reinterpret_cast<void**>(&source)))
		{
			if (source)
			{
				LOGI("Process source");
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
