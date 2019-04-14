#pragma once
#include "android_native_app_glue.h"
#include <map>
#include <list>

class MotionEvent
{
public:
    bool isMultitouch(AInputEvent *event);

    void addPoint(int32_t id);

    void removePoint(int32_t id);

    glm::vec2 getMotionDelta(AInputEvent *event, int32_t id);

    float  getZoomDelta(AInputEvent *event);

private:
    std::list<int32_t> idList;

    std::map<int32_t, glm::vec2> points;

    glm::vec2 getNewPoint(AInputEvent *event, int32_t id);
};

