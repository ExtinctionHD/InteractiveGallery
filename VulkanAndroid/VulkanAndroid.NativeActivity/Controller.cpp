#include "Controller.h"
#include <glm/gtx/rotate_vector.hpp>
#include "utils.h"

Controller::Controller(glm::vec3 target, glm::vec3 position)
    : target(target),
      radius(glm::distance(target, position)),
      motionDelta(glm::vec2(0.0f))
{
    const glm::vec3 view = glm::normalize(target - position);

    angle.x = glm::radians(90.0f) + std::atan2(view.z, view.x);
    angle.y = glm::degrees(glm::asin(view.y));
}

Camera::Location Controller::getLocation() const
{
    glm::vec3 hAxis;

    const glm::vec3 view = axis::rotate(-axis::Z, angle, &hAxis);
    const glm::vec3 up = glm::normalize(glm::cross(-view, hAxis));

    return Camera::Location{ target + view * radius, target, up };
}

glm::vec2 Controller::getCoordinates(float earthAngle) const
{
    float longitude = std::fmod(90.0f + angle.x - earthAngle, 360.0f);

    if (longitude > 180.0f)
    {
        longitude = -(360.0f - longitude);
    }
    if (longitude < -180.0f)
    {
        longitude = 360.0f + longitude;
    }

    return glm::vec2(longitude, angle.y);
}

float Controller::getRadius()
{
    return radius;
}

void Controller::setMotionDelta(glm::vec2 newDelta)
{
    newDelta.x = -newDelta.x;
    motionDelta = (motionDelta + newDelta) / 2.0f;
}

void Controller::setZoomDelta(float newDelta)
{
    newDelta = -newDelta;
    zoomDelta = (zoomDelta + newDelta) / 2.0f;
}

void Controller::update(float deltaSec)
{
    angle += radius / SENSITIVITY * motionDelta * deltaSec;
    radius += SENSITIVITY * zoomDelta * deltaSec;

    const glm::vec2 motionFading = FADING * motionDelta * deltaSec;
    motionDelta.x = glm::abs(motionDelta.x) > glm::abs(motionFading.x) ? motionDelta.x - motionFading.x : 0.0f;
    motionDelta.y = glm::abs(motionDelta.y) > glm::abs(motionFading.y) ? motionDelta.y - motionFading.y : 0.0f;

    const float zoomFading = FADING * zoomDelta * deltaSec;
    zoomDelta = glm::abs(zoomDelta) > glm::abs(zoomFading) ? zoomDelta - zoomFading : 0.0f;

    // restricts Y angle
    const float maxY = 85.0f;
    const float absY = glm::abs(angle.y);
    angle.y = absY > maxY ? maxY * angle.y / absY : angle.y;

    // TODO: add zoom restrictions 
}
