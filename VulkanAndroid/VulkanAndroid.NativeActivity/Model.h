#pragma once
#include "Buffer.h"

class Model
{
public:
    virtual ~Model();

    Buffer* getTransformationBuffer() const;

    void rotate(float angle, glm::vec3 axis);

    void setPosition(glm::vec3 position);

protected:
    Model(Device *device);

private:
    glm::mat4 transformation;

    Buffer *transformationBuffer;
};

