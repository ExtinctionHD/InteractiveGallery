#include "ActivityManager.h"
#include <fstream>

void ActivityManager::init(ANativeActivity *activity)
{
    ActivityManager::activity = activity;
}

std::vector<uint8_t> ActivityManager::readFromExternalStorage(const std::string &path)
{
    const std::string fullPath = getExternalStoragePath() + path;
    std::ifstream file(fullPath, std::ios::ate | std::ios::binary);

    LOGA(file.is_open());

    const size_t fileSize = size_t(file.tellg());
    std::vector<uint8_t> buffer(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    file.close();

    LOGD("Load file from external storage: [%s]", fullPath.c_str());

    return buffer;
}

std::vector<uint8_t> ActivityManager::readFromAssets(const std::string &path)
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

std::string ActivityManager::getExternalStoragePath()
{
    LOGA(activity);

    JNIEnv *env;
    activity->vm->AttachCurrentThread(&env, nullptr);

    jclass clsEnv = env->FindClass("android/os/Environment");
    jmethodID midGetExternalStorageDirectory = env->GetStaticMethodID(clsEnv, "getExternalStorageDirectory", "()Ljava/io/File;");
    jobject objFile = env->CallStaticObjectMethod(clsEnv, midGetExternalStorageDirectory);
    jclass clsFile = env->FindClass("java/io/File");
    jmethodID midGetPath = env->GetMethodID(clsFile, "getPath", "()Ljava/lang/String;");
    jstring strPath = jstring(env->CallObjectMethod(objFile, midGetPath));

    const char* path = env->GetStringUTFChars(strPath, nullptr);
    std::string result(env->GetStringUTFChars(strPath, nullptr));
    result.append("/");

    env->ReleaseStringUTFChars(strPath, path);
    activity->vm->DetachCurrentThread();

    return result;
}

ANativeActivity* ActivityManager::activity = nullptr;