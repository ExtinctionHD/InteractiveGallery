#pragma once
#include "Buffer.h"

class Model
{
public:
    virtual ~Model();

    Buffer* getTransformationBuffer() const;

protected:
    Model(Device *device);

private:
    glm::mat4 transformation;

    Buffer *transformationBuffer;
};

