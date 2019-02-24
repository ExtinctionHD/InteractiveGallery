#pragma once
#include "Camera.h"

class Controller
{
public:
    Controller(glm::vec3 target, glm::vec3 position);

    Camera::Location getLocation() const;

    void setDelta(glm::vec2 delta);

    void update(float deltaSec);

private:
    const float SENSITIVITY = 5.0f;

    glm::vec3 target;

    float radius;

    glm::vec2 angle;

    glm::vec2 delta;
};

