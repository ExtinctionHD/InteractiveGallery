#include "Model.h"
#include <glm/gtx/transform.hpp>

Model::~Model()
{
    delete transformationBuffer;
}

Buffer* Model::getTransformationBuffer() const
{
    return transformationBuffer;
}

void Model::rotate(float angle, glm::vec3 axis)
{
    transformation = glm::rotate(transformation, glm::radians(angle), axis);
    transformationBuffer->updateData(&transformation);
}

Model::Model(Device *device) : transformation(glm::mat4(1.0f))
{
    transformationBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4));
    transformationBuffer->updateData(&transformation);
}
