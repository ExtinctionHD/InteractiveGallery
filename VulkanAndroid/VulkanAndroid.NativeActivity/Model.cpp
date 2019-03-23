#include "Model.h"
#include <glm/gtx/transform.hpp>

Model::~Model()
{
    delete transformationBuffer;
}

DescriptorInfo Model::getTransformationBufferInfo() const
{
    return transformationBuffer->getUniformBufferInfo();
}

glm::mat4 Model::getTransformation() const
{
    return transformation;
}

void Model::setTransformation(glm::mat4 transformation)
{
    this->transformation = transformation;
    transformationBuffer->updateData(&transformation);
}

Model::Model(Device *device) : transformation(glm::mat4(1.0f))
{
    transformationBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4));
    transformationBuffer->updateData(&transformation);
}
