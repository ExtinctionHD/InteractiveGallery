//
// pch.h
// Header for standard system include files.
//
// Used by the build system to generate the precompiled header. Note that no
// pch.cpp is needed and the pch.h is automatically included in all cpp files
// that are part of the project
//

#pragma once

#include <jni.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>
#include <android/log.h>

#include <cassert>
#include <set>
#include <string>
#include <vector>

#include "Macro.h"

#define VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan_wrapper.h"
