#pragma once

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "VulkanAndroid", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "VulkanAndroid", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "VulkanAndroid", __VA_ARGS__))
#ifndef NDEBUG
    #define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, "VulkanAndroid", __VA_ARGS__))
#else
    #define LOGV(...)  ((void)0)
#endif

#define CALL_VK(func) assert(VK_SUCCESS == (func))
