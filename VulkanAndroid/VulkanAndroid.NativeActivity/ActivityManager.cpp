#include "ActivityManager.h"
#include <fstream>
#include <dirent.h>
#include <algorithm>

void ActivityManager::init(ANativeActivity *activity)
{
    ActivityManager::activity = activity;
}

std::vector<uint8_t> ActivityManager::read(const std::string &path)
{
    const std::string fullPath = getExternalStoragePath() + path;
    std::ifstream file(fullPath, std::ios::ate | std::ios::binary);

    LOGA(file.is_open());

    const auto fileSize = file.tellg();
    std::vector<uint8_t> buffer(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    file.close();

    LOGD("Load file from external storage: [%s]", fullPath.c_str());

    return buffer;
}

std::vector<uint8_t> ActivityManager::readAsset(const std::string &path)
{
    LOGA(activity);

    AAsset *asset = AAssetManager_open(activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);

    LOGA(asset);

    const auto size = AAsset_getLength(asset);
    std::vector<uint8_t> buffer(size);
    AAsset_read(asset, buffer.data(), size);

    AAsset_close(asset);

    LOGD("Load file from assets: [%s]", path.c_str());

    return buffer;
}

std::vector<std::string> ActivityManager::getFilePaths(
    const std::string &path,
    const std::vector<std::string> &extensions)
{
    const std::string fullPath = getExternalStoragePath() + path;
    std::vector<std::string> fileNames;

    const auto dir = opendir(fullPath.c_str());
    if (dir)
    {
        auto entry = readdir(dir);
        while (entry)
        {
            if (entry->d_type == DT_REG)
            {
                std::string fileName = entry->d_name;

                for (const auto &extension : extensions)
                {
                    if (fileName.find(extension, fileName.length() - extension.length()) != std::string::npos)
                    {
                        fileNames.push_back(path + fileName);
                    }
                }
            }
            entry = readdir(dir);
        }
    }

    return fileNames;
}

std::string ActivityManager::getExternalStoragePath()
{
    LOGA(activity);

    static std::string externalStoragePath{};

    if (externalStoragePath.empty())
    {
        JNIEnv *env;
        activity->vm->AttachCurrentThread(&env, nullptr);

        jclass clsEnv = env->FindClass("android/os/Environment");
        jmethodID midGetExternalStorageDirectory = env->GetStaticMethodID(clsEnv, "getExternalStorageDirectory", "()Ljava/io/File;");
        jobject objFile = env->CallStaticObjectMethod(clsEnv, midGetExternalStorageDirectory);
        jclass clsFile = env->FindClass("java/io/File");
        jmethodID midGetPath = env->GetMethodID(clsFile, "getPath", "()Ljava/lang/String;");
        jstring strPath = jstring(env->CallObjectMethod(objFile, midGetPath));

        const char* path = env->GetStringUTFChars(strPath, nullptr);
        externalStoragePath = std::string(env->GetStringUTFChars(strPath, nullptr));
        externalStoragePath.append("/");

        env->ReleaseStringUTFChars(strPath, path);
        activity->vm->DetachCurrentThread();
    }

    return externalStoragePath;
}

ANativeActivity* ActivityManager::activity = nullptr;