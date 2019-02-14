#pragma once

namespace math
{
    uint32_t ceilLog2(uint32_t x);
}

namespace window
{
    VkExtent2D getExtent(ANativeWindow *window);
}