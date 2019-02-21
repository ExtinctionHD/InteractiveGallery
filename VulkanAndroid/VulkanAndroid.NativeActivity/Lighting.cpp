#include "Lighting.h"

Lighting::Lighting(Device *device, Attributes attributes) : attributes(attributes)
{
    buffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Attributes));
    buffer->updateData(&attributes);
}

Lighting::~Lighting()
{
    delete buffer;
}

Buffer* Lighting::getBuffer() const
{
    return buffer;
}

void Lighting::update(glm::vec3 cameraPos)
{
    attributes.cameraPos = cameraPos;
    buffer->updateData(&cameraPos, offsetof(Attributes, cameraPos), sizeof(glm::vec3));
}
