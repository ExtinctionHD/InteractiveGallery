#include "MotionEvent.h"
#include <algorithm>

bool MotionEvent::isMultitouch(AInputEvent *event)
{
    return AMotionEvent_getPointerCount(event) > 1;
}

void MotionEvent::addPoint(int32_t id)
{
    if (std::find(idList.begin(), idList.end(), id) == idList.end())
    {
        idList.push_back(id);
    }
}

void MotionEvent::removePoint(int32_t id)
{
    idList.remove(id);
}

glm::vec2 MotionEvent::getMotionDelta(AInputEvent *event, int32_t id)
{
    glm::vec2 &currentPoint = points[id];

    const glm::vec2 newPoint = getNewPoint(event, id);

    const glm::vec2 delta = newPoint - currentPoint;

    currentPoint = newPoint;

    return delta;
}

float MotionEvent::getZoomDelta(AInputEvent *event, int32_t id)
{
    LOGI("%d, %d", idList.front(), *++idList.begin());

    glm::vec2 &firstPoint = points[idList.front()];
    glm::vec2 &secondPoint = points[*++idList.begin()];

    LOGI("Id: %d", id);
    LOGI("First x: %f, y: %f", firstPoint.x, firstPoint.y);
    LOGI("Second x: %f, y: %f", secondPoint.x, secondPoint.y);

    const float currentDelta = glm::distance(firstPoint, secondPoint);

    LOGI("Current delta: %f", currentDelta);

    points[id] = getNewPoint(event, id);

    return glm::distance(firstPoint, secondPoint) - currentDelta;
}

glm::vec2 MotionEvent::getNewPoint(AInputEvent *event, int32_t id)
{
    glm::vec2 newPoint;
    newPoint.x = AMotionEvent_getX(event, id);
    newPoint.y = AMotionEvent_getY(event, id);

    return newPoint;
}
