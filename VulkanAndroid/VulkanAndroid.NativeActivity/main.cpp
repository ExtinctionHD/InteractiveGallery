#include "android_native_app_glue.h"
#include "Application.h"

void android_main(android_app *app) 
{
    Application application(app);

    application.mainLoop();
}
