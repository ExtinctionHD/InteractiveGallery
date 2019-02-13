#pragma once

#ifndef NDEBUG
    #define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, "VulkanAndroid", __VA_ARGS__))
    #define LOGD(...)  ((void)__android_log_print(ANDROID_LOG_DEBUG, "VulkanAndroid", __VA_ARGS__))
#else
    #define LOGV(...)  ((void)0)
    #define LOGD(...)  ((void)0)
#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "VulkanAndroid", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "VulkanAndroid", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "VulkanAndroid", __VA_ARGS__))
#define LOGF(...) ((void)__android_log_print(ANDROID_LOG_FATAL, "VulkanAndroid", __VA_ARGS__))

#ifndef __PRETTY_FUNCTION__
    #define __PRETTY_FUNCTION__ __func__
#endif

#define CALL_VK(vkFunc)                                                     \
    if(VK_SUCCESS != (vkFunc))                                              \
    {                                                                       \
        LOGF("Vulkan call failed. File [%s], line %d", __FILE__, __LINE__); \
        assert(false);                                                      \
    }

#define ERROR(...)      \
	LOGF(__VA_ARGS__);  \
	assert(false)

#define LOGA(e, ...)        \
    if (!(e))               \
    {                       \
        LOGF(__VA_ARGS__);  \
    }                       \
    assert(e)
