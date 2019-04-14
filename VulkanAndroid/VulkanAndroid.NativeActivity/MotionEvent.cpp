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

    glm::vec2 delta = newPoint - currentPoint;
    if (currentPoint == glm::vec2(0.0f) || newPoint == glm::vec2(0.0f))
    {
        delta = glm::vec2(0.0f);
    }

    currentPoint = newPoint;

    return delta;
}

float MotionEvent::getZoomDelta(AInputEvent *event)
{
    if (idList.size() < 2)
    {
        return 0.0f;
    }

    const int32_t firstId = idList.front();
    const int32_t secondId = *++idList.begin();

    glm::vec2 &firstPoint = points[firstId];
    glm::vec2 &secondPoint = points[secondId];

    const float currentDelta = glm::distance(firstPoint, secondPoint);

    points[firstId] = getNewPoint(event, firstId);
    points[secondId] = getNewPoint(event, secondId);

    return glm::distance(firstPoint, secondPoint) - currentDelta;
}

glm::vec2 MotionEvent::getNewPoint(AInputEvent *event, int32_t id)
{
    glm::vec2 newPoint;
    newPoint.x = AMotionEvent_getX(event, id);
    newPoint.y = AMotionEvent_getY(event, id);

    return newPoint;
}
