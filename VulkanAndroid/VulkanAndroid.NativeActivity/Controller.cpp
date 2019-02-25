#include "Controller.h"
#include <glm/gtx/rotate_vector.hpp>
#include "utils.h"

Controller::Controller(glm::vec3 target, glm::vec3 position) : target(target), radius(glm::distance(target, position))
{
    const glm::vec3 view = normalize(target - position);
    const glm::vec3 horizontal = normalize(glm::vec3(view.x, 0.0f, view.z ));

    if (horizontal.x >= 0.0f)
    {
        if (horizontal.z >= 0.0f)
        {
            // first quarter
            angle.x = 360.0f - glm::degrees(glm::asin(horizontal.x));
        }
        else
        {
            // second quarter
            angle.x = 180.0f + glm::degrees(glm::asin(horizontal.x));
        }
    }
    else
    {
        if (horizontal.z >= 0.0f)
        {
            // third quarter
            angle.x = glm::degrees(glm::asin(-horizontal.x));
        }
        else
        {
            // fourth quarter
            angle.x = 90.0f + glm::degrees(glm::asin(-horizontal.x));
        }
    }

    angle.y = glm::degrees(glm::asin(view.y));
}

Camera::Location Controller::getLocation() const
{
    const glm::vec3 vAxis = -vector::Y;

    glm::vec3 view = rotate(-vector::Z, glm::radians(angle.x), vAxis);
    view = normalize(view);

    glm::vec3 hAxis = cross(view, vAxis);
    hAxis = normalize(hAxis);
    view = rotate(view, glm::radians(angle.y), hAxis);

    view = normalize(view);
    const glm::vec3 up = normalize(cross(-view, hAxis));

    return Camera::Location{ target + view * radius, target, up };
}

void Controller::setDelta(glm::vec2 delta)
{
    delta.x = -delta.x;
    this->delta = (this->delta + delta) / 2.0f;

    LOGD("Delta x: %f, y: %f.", delta.x, delta.y);
}

void Controller::update(float deltaSec)
{
    angle += SENSITIVITY * delta * deltaSec;

    const glm::vec2 fading = FADING * delta * deltaSec;
    
    delta.x = glm::abs(delta.x) > glm::abs(fading.x) ? delta.x - fading.x : 0.0f;
    delta.y = glm::abs(delta.y) > glm::abs(fading.y) ? delta.y - fading.y : 0.0f;

    const float maxY = 85.0f;
    const float absY = glm::abs(angle.y);

    angle.y = absY > maxY ? maxY * angle.y / absY : angle.y;
}
