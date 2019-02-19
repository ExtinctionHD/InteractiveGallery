#include "Model.h"

Model::~Model()
{
    delete transformationBuffer;
}

Buffer* Model::getTransformationBuffer() const
{
    return transformationBuffer;
}

Model::Model(Device *device) : transformation(glm::mat4(1.0f))
{
    transformationBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4));
    transformationBuffer->updateData(&transformation);
}
