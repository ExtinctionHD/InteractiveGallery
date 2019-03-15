#include "PhotoCard.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "sphere.h"
#include "utils.h"

PhotoCard::PhotoCard(Device *device, const std::string &texturePath) : Model(device)
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

void PhotoCard::setLocation(float latitude, float longitude)
{
    position = axis::rotate(-axis::X, glm::vec2(longitude, latitude), nullptr) * sphere::R;

    glm::mat4 transformation = translate(glm::mat4(1.0f), position);
    transformation = glm::scale(transformation, glm::vec3(10.0f, 5.6f, 1.0f));

    setTransformation(transformation);
}
