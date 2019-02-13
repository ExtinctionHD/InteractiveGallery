#include "AssetManager.h"

void AssetManager::setManager(AAssetManager *manager)
{
    AssetManager::manager = manager;
}

std::vector<uint8_t> AssetManager::getBytes(const std::string &path)
{
    LOGA(manager, "Asset manager undefined.");

    AAsset *asset = AAssetManager_open(manager, path.c_str(), AASSET_MODE_UNKNOWN);

    LOGA(manager, "Asset %s not found.", path.c_str());

    const auto size = AAsset_getLength(asset);
    std::vector<uint8_t> buffer(size);
    AAsset_read(asset, buffer.data(), size);

    AAsset_close(asset);

    return buffer;
}

AAssetManager* AssetManager::manager = nullptr;
