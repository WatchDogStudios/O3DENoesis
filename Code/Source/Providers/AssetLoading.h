#pragma once

#include <AzCore/Asset/AssetManager.h>
#include <Runtime/EngineHooks.h>

namespace NoesisGUI
{
    // Noesis provider calls are synchronous by contract; the view's first frame waits for its assets.
    template<class T>
    AZ::Data::Asset<T> LoadAssetBlocking(const AZStd::string& productPath)
    {
        const AZ::Data::AssetId assetId = EngineHooks::FindAssetIdByPath(productPath);
        if (!assetId.IsValid())
        {
            AZ_Warning("NoesisGUI", false, "No asset for '%s'", productPath.c_str());
            return {};
        }
        AZ::Data::Asset<T> asset = AZ::Data::AssetManager::Instance().GetAsset<T>(assetId, AZ::Data::AssetLoadBehavior::PreLoad);
        asset.BlockUntilLoadComplete();
        AZ_Warning("NoesisGUI", asset.IsReady(), "Failed to load '%s'", productPath.c_str());
        return asset.IsReady() ? asset : AZ::Data::Asset<T>{};
    }
}
