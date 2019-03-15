#include "Model.h"
#include <glm/gtx/transform.hpp>

Model::~Model()
{
    delete transformationBuffer;
}

glm::mat4 Model::getTransformation() const
{
    return transformation;
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

void Model::setPosition(glm::vec3 position)
{
    transformation = translate(glm::mat4(1.0f), position);
    transformationBuffer->updateData(&transformation);
}

void Model::setScale(glm::vec3 scale)
{
    transformation = glm::scale(glm::mat4(1.0f), scale);
    transformationBuffer->updateData(&transformation);
}

Model::Model(Device *device) : transformation(glm::mat4(1.0f))
{
    transformationBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4));
    transformationBuffer->updateData(&transformation);
}

void Model::setTransformation(glm::mat4 transformation)
{
    this->transformation = transformation;
    transformationBuffer->updateData(&transformation);
}
