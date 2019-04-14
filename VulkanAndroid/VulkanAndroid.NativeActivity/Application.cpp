#include "Application.h"
#include "ActivityManager.h"
#include "MotionEvent.h"

Application::Application(android_app *app) : app(app)
{
    ActivityManager::init(app->activity);

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
        static MotionEvent motionEvent;

        const int32_t action = AMotionEvent_getAction(event);
        const int32_t actionType = action & AMOTION_EVENT_ACTION_MASK;
        const int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        const int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);

        switch (actionType)
        {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
            engine->activateGallery();
            motionEvent.addPoint(pointerId);
            motionEvent.getMotionDelta(event, pointerId);
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            if (motionEvent.isMultitouch(event))
            {
                engine->handleZoom(motionEvent.getZoomDelta(event));
            }
            else
            {
                engine->handleMotion(motionEvent.getMotionDelta(event, pointerId));
            }
            break;
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP:
            motionEvent.removePoint(pointerId);
            break;
        default:
            break;
        }
    }

    return 0;
}
