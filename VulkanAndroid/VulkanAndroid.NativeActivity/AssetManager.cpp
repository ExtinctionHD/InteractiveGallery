#include "AssetManager.h"

void AssetManager::setManager(AAssetManager *manager)
{
    AssetManager::manager = manager;
}

std::vector<uint8_t> AssetManager::getBytes(const std::string &path)
{
    LOGA(manager);

    AAsset *asset = AAssetManager_open(manager, path.c_str(), AASSET_MODE_UNKNOWN);

    LOGA(asset);

    const auto size = AAsset_getLength(asset);
    std::vector<uint8_t> buffer(size);
    AAsset_read(asset, buffer.data(), size);

    AAsset_close(asset);

    return buffer;
}

AAssetManager* AssetManager::manager = nullptr;
