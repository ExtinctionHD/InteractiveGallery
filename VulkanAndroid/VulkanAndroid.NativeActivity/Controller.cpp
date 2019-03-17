#include "Controller.h"
#include <glm/gtx/rotate_vector.hpp>
#include "utils.h"

Controller::Controller(glm::vec3 target, glm::vec3 position)
    : target(target),
      radius(distance(target, position)),
      delta(glm::vec2(0.0f))
{
    const glm::vec3 view = normalize(target - position);

    angle.x = glm::radians(90.0f) + std::atan2(view.z, view.x);
    angle.y = glm::degrees(glm::asin(view.y));
}

Camera::Location Controller::getLocation() const
{
    glm::vec3 hAxis;

    const glm::vec3 view = axis::rotate(-axis::Z, angle, &hAxis);
    const glm::vec3 up = normalize(cross(-view, hAxis));

    return Camera::Location{ target + view * radius, target, up };
}

glm::vec2 Controller::getCoordinates(float earthAngle) const
{
    float longitude = std::fmod(90.0f + angle.x - earthAngle, 360.0f);
    if (longitude > 180.0f)
    {
        longitude = 360.0f - longitude;
    }
    if (longitude < -180.0f)
    {
        longitude = 360.0f + longitude;
    }
    return glm::vec2(longitude, angle.y);
}

void Controller::setDelta(glm::vec2 newDelta)
{
    newDelta.x = -newDelta.x;
    delta = (delta + newDelta) / 2.0f;
}

void Controller::update(float deltaSec)
{
    angle += SENSITIVITY * delta * deltaSec;

    const glm::vec2 fading = FADING * delta * deltaSec;
    
    delta.x = glm::abs(delta.x) > glm::abs(fading.x) ? delta.x - fading.x : 0.0f;
    delta.y = glm::abs(delta.y) > glm::abs(fading.y) ? delta.y - fading.y : 0.0f;

    // restricts Y angle
    const float maxY = 85.0f;
    const float absY = glm::abs(angle.y);
    angle.y = absY > maxY ? maxY * angle.y / absY : angle.y;
}
