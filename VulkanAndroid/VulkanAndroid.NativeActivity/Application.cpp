#include "Application.h"
#include "ActivityManager.h"

Application::Application(android_app *app) : app(app)
{
    ActivityManager::init(app->activity);

    ActivityManager::getFileNames("Download/", { ".jpg", ".jpeg" });

    app->userData = &engine;
    app->onAppCmd = handleAppCommand;
    app->onInputEvent = handleAppInput;
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
                LOGI("Applicatoin destroy requested.");
                return;
            }
        }

        engine.drawFrame();
    }
}

void Application::handleAppCommand(android_app *app, int32_t cmd)
{
    LOGA(app->userData);
    auto engine = reinterpret_cast<Engine*>(app->userData);

    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
        LOGD("APP_CMD_INIT_WINDOW");
        LOGA(app->window);
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

int32_t Application::handleAppInput(android_app *app, AInputEvent *event)
{
    LOGA(app->userData);
    auto engine = reinterpret_cast<Engine*>(app->userData);

    if (!engine->onPause() && AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        static glm::vec2 last;

        const int32_t action = AMotionEvent_getAction(event);

        switch (action)
        {
        case AMOTION_EVENT_ACTION_DOWN:
            LOGD("DOWN");
            last.x = AMotionEvent_getX(event, 0);
            last.y = AMotionEvent_getY(event, 0);
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            glm::vec2 current;
            current.x = AMotionEvent_getX(event, 0);
            current.y = AMotionEvent_getY(event, 0);
            engine->handleMotion(current - last);
            last = current;
            break;
        case AMOTION_EVENT_ACTION_UP:
            LOGD("UP");
            break;
        default:
            LOGD("DEFAULT %d", action);
            break;;
        }
    }

    return 0;
}
