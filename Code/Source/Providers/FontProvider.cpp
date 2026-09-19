#include <Providers/FontProvider.h>

#include <NoesisGUI/NoesisFontAsset.h>
#include <Providers/AssetLoading.h>
#include <Providers/AssetStream.h>
#include <Providers/AssetUri.h>

#include <NsCore/String.h>
#include <NsGui/Uri.h>

namespace NoesisGUI
{
    void FontProvider::ScanFolder(const Noesis::Uri& folder)
    {
        Noesis::FixedString<512> path;
        folder.GetPath(path);
        const AZStd::string folderPath = AssetUri::ToProductPath(path.Str(), {});
        // CachedFontProvider scans each folder once, so the catalog walk happens once per font folder.
        EngineHooks::EnumerateProductPaths(azrtti_typeid<NoesisFontAsset>(),
            [&](const AZStd::string& productPath)
            {
                if (AssetUri::FolderOf(productPath) == folderPath)
                {
                    RegisterFont(folder, AssetUri::SourceFileName(productPath, NoesisFontAsset::Extension).c_str());
                }
            });
    }

    Noesis::Ptr<Noesis::Stream> FontProvider::OpenFont(const Noesis::Uri& folder, const char* filename) const
    {
        Noesis::FixedString<512> path;
        folder.GetPath(path);
        // Normalize the folder first (as ScanFolder does) so a trailing slash from Noesis doesn't
        // produce a double slash that misses the catalog. ToProductPath also trims any leading
        // slash, so an empty folder joins cleanly too.
        const AZStd::string folderPath = AssetUri::ToProductPath(path.Str(), {});
        const AZStd::string sourcePath = AZStd::string::format("%s/%s", folderPath.c_str(), filename);
        AZ::Data::Asset<NoesisFontAsset> asset = LoadAssetBlocking<NoesisFontAsset>(AssetUri::ToProductPath(sourcePath, NoesisFontAsset::Extension));
        if (!asset)
        {
            return nullptr;
        }
        const AZStd::vector<AZ::u8>& bytes = asset->m_data;
        return *new AssetStream(asset, bytes);
    }
}
