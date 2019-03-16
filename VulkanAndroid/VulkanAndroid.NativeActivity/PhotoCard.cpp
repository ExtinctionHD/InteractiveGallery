#include "PhotoCard.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "sphere.h"
#include "utils.h"

PhotoCard::PhotoCard(Device *device, const std::string &texturePath, Earth *earth) : Model(device), earth(earth)
{
    texture = new TextureImage(
        device,
        { texturePath + TEXTURE_FILE },
        false);
    texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

}

PhotoCard::~PhotoCard()
{
    delete texture;
}

TextureImage * PhotoCard::getTexture() const
{
    return texture;
}

void PhotoCard::update(float latitude, float longitude, glm::vec3 cameraPos, glm::vec3 cameraRight)
{
    position = axis::rotate(-axis::X, glm::vec2(longitude + earth->getAngle(), latitude), nullptr) * sphere::R;
    
    const glm::vec3 direction = normalize(cameraPos - position);
    const glm::vec2 angle( glm::radians(90.0f) + std::atan2(direction.z, direction.x), glm::asin(direction.y));

    glm::mat4 transformation = translate(glm::mat4(1.0f), position);
    transformation = rotate(transformation, angle.y, cameraRight);
    transformation = rotate(transformation, angle.x, -axis::Y);
    transformation = scale(transformation, glm::vec3(5.0f, 2.8f, 1.0f));

    setTransformation(transformation);
}
