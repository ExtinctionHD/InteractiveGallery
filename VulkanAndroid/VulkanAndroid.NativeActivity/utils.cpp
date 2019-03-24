#include "utils.h"
#include <android/native_window.h>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>

uint32_t math::ceilLog2(uint32_t x)
{
    static const unsigned long long T[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
    };

    int y = (((x & (x - 1)) == 0) ? 0 : 1);
    int j = 32;

    for (unsigned long long i : T)
    {
        const int k = (((x & i) == 0) ? 0 : j);
        y += k;
        x >>= k;
        j >>= 1;
    }

    return y;
}

VkExtent2D window::getExtent(ANativeWindow *window)
{
    return VkExtent2D{
        uint32_t(ANativeWindow_getWidth(window)),
        uint32_t(ANativeWindow_getHeight(window))
    };
}

glm::vec3 axis::rotate(glm::vec3 baseAxis, glm::vec2 angle, glm::vec3 *outHorizontalAxis)
{
    glm::vec3 result = glm::rotate(baseAxis, glm::radians(angle.x), -Y);
    result = glm::normalize(result);

    glm::vec3 hAxis = glm::cross(result, -Y);
    hAxis = glm::normalize(hAxis);
    result = glm::rotate(result, glm::radians(angle.y), hAxis);

    if (outHorizontalAxis)
    {
        *outHorizontalAxis = hAxis;
    }

    return glm::normalize(result);
}

std::string file::getFileName(std::string path)
{
    std::replace(path.begin(), path.end(), '\\', '/');

    auto index = path.find_last_of('/');
    if (index != std::string::npos)
    {
        path = path.substr(index + 1);
    }

    index = path.find_last_of('.');
    if (index != std::string::npos)
    {
        path = path.substr(0, index);
    }

    return path;
}
