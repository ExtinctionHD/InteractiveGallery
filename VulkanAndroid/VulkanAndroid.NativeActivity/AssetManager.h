#pragma once
#include "android_native_app_glue.h"

class AssetManager
{
public:
    static void setManager(AAssetManager *manager);

    static std::vector<uint8_t> getBytes(const std::string &path);

private:
    static AAssetManager *manager;
};

