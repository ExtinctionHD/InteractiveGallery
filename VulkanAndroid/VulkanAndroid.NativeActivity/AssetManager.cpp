#include "AssetManager.h"

void AssetManager::setManager(AAssetManager *manager)
{
    AssetManager::manager = manager;
}

std::vector<uint8_t> AssetManager::getBytes(const std::string &fileName)
{
    LOGA(manager, "Asset manager not defined");

    AAsset *asset = AAssetManager_open(manager, fileName.c_str(), AASSET_MODE_UNKNOWN);

    LOGA(manager, "Asset %s not found", fileName.c_str());

    const auto size = AAsset_getLength(asset);
    std::vector<uint8_t> buffer(size);
    AAsset_read(asset, buffer.data(), size);

    AAsset_close(asset);

    return buffer;
}

AAssetManager* AssetManager::manager = nullptr;
