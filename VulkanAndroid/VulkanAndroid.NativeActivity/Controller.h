#pragma once
#include "Camera.h"

class Controller
{
public:
    Controller(glm::vec3 target, glm::vec3 position);

    Camera::Location getLocation() const;

    glm::vec2 getCoordinates(float earthAngle) const;

    float getRadius();

    void setMotionDelta(glm::vec2 newDelta);

    void setZoomDelta(float newDelta);

    void update(float deltaSec);

private:
    const float ROTATION_SENSITIVITY = 5.0f;
    const float ROTATION_FADING = 2.0f;

    const float ZOOM_SENSITIVITY = 1.5f;
    const float ZOOM_FADING = 3.0f;

    glm::vec3 target;

    float radius;

    glm::vec2 angle;

    glm::vec2 motionDelta = glm::vec2(0.0f);

    float zoomDelta = 0.0f;
};

