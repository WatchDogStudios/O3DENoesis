#include <Providers/XamlProvider.h>

#include <NoesisGUI/NoesisXamlAsset.h>
#include <Providers/AssetLoading.h>
#include <Providers/AssetStream.h>
#include <Providers/AssetUri.h>

#include <NsCore/String.h>
#include <NsGui/Uri.h>

namespace NoesisGUI
{
    XamlProvider::XamlProvider(ReloadRegistry* registry)
        : m_registry(registry)
    {
    }

    Noesis::Ptr<Noesis::Stream> XamlProvider::LoadXaml(const Noesis::Uri& uri)
    {
        Noesis::FixedString<512> path;
        uri.GetPath(path);
        AZ::Data::Asset<NoesisXamlAsset> asset = LoadAssetBlocking<NoesisXamlAsset>(AssetUri::ToProductPath(path.Str(), NoesisXamlAsset::Extension));
        if (!asset)
        {
            return nullptr;
        }
        if (m_registry)
        {
            m_registry->Record(asset.GetId(), ReloadKind::Xaml, uri.Str());
        }
        const AZStd::vector<AZ::u8>& bytes = asset->m_data;
        return *new AssetStream(asset, bytes);
    }
}
