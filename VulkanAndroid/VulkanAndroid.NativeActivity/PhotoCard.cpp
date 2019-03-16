#include "PhotoCard.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "sphere.h"
#include "utils.h"

PhotoCard::PhotoCard(
    Device *device,
    const std::string &texturePath,
    Earth *earth,
    Camera *camera,
    Controller *controller)
    : Model(device),
      earth(earth),
      camera(camera),
      controller(controller)
{
    texture = new TextureImage(
        device,
        { texturePath + TEXTURE_FILE },
        false);
    texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    opacityBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float));
}

PhotoCard::~PhotoCard()
{
    delete opacityBuffer;
    delete texture;
}

TextureImage* PhotoCard::getTexture() const
{
    return texture;
}

Buffer* PhotoCard::getOpacityBuffer() const
{
    return opacityBuffer;
}

void PhotoCard::update()
{
    const glm::vec2 coordinates = controller->getCoordinates(earth->getAngle());
    const float distance = loopDistance(coordinates, COORDINATES);

    float opacity = 0.0f;
    if (distance < MAX_DISTANCE)
    {
        const float minDistance = MAX_DISTANCE / 2.0f;

        opacity = 1.0f - (distance - minDistance) / (MAX_DISTANCE - minDistance);

        opacity = opacity > 1.0f ? 1.0f : opacity;
        opacity = std::pow(opacity, 0.5f);
    }

    opacityBuffer->updateData(&opacity);

    if (opacity > 0.0f)
    {
        position = axis::rotate(-axis::X, glm::vec2(COORDINATES.x + earth->getAngle(), COORDINATES.y), nullptr) * sphere::R;
        const glm::vec3 direction = normalize(camera->getPosition() - position);
        const glm::vec2 angle(glm::radians(90.0f) + std::atan2(direction.z, direction.x), glm::asin(direction.y));

        glm::mat4 transformation = translate(glm::mat4(1.0f), position);
        transformation = rotate(transformation, angle.y, camera->getRight());
        transformation = rotate(transformation, angle.x, -axis::Y);
        transformation = scale(transformation, glm::vec3(5.0f, 2.8f, 1.0f));

        setTransformation(transformation);
    }
}

float PhotoCard::loopDistance(glm::vec2 a, glm::vec2 b)
{
    float result;

    if (glm::abs(a.x - b.x) < 180.0f)
    {
        result = distance(a, b);
    }
    else
    {
        if (a.x < b.x)
        {
            a.x += 360.0f;
        }
        else
        {
            b.x += 360.0f;
        }
        result = distance(a, b);
    }

    return result;
}
