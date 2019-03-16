#pragma once
#include "android_native_app_glue.h"

class ActivityManager
{
public:
    static void init(ANativeActivity *activity);

    static std::vector<uint8_t> readFromExternalStorage(const std::string &path);

    static std::vector<uint8_t> readFromAssets(const std::string &path);

private:
    static ANativeActivity *activity;

    static std::string getExternalStoragePath();
};

