#pragma once
#include "android_native_app_glue.h"

class ActivityManager
{
public:
    static void init(ANativeActivity *activity);

    static std::vector<uint8_t> read(const std::string &path);

    static std::vector<uint8_t> readAsset(const std::string &path);

    static std::vector<std::string> getFileNames(const std::string &path, const std::vector<std::string> &extensions);

private:
    static ANativeActivity *activity;

    static std::string getExternalStoragePath();
};

