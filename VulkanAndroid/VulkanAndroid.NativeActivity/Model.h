#pragma once
#include "Buffer.h"

class Model
{
public:
    virtual ~Model();

    Buffer* getTransformationBuffer() const;

    glm::mat4 getTransformation() const;

    void setTransformation(glm::mat4 transformation);

protected:
    Model(Device *device);

private:
    glm::mat4 transformation;

    Buffer *transformationBuffer;
};

