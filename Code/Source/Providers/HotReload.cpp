#include <Providers/HotReload.h>

#include <Providers/TextureProvider.h>
#include <Providers/XamlProvider.h>

#include <NsGui/Uri.h>

namespace NoesisGUI
{
    HotReload::HotReload(ReloadRegistry& registry, XamlProvider& xaml, TextureProvider& textures)
        : m_registry(registry)
        , m_xaml(xaml)
        , m_textures(textures)
    {
        m_catalogConnection = EngineHooks::ConnectAssetChanged([this](const AZ::Data::AssetId& assetId) { m_registry.MarkChanged(assetId); });
    }

    HotReload::~HotReload() = default;

    // ponytail: font hot reload not wired; FontProvider::RaiseFontChanged needs family/weight/stretch/style per face.
    void HotReload::Flush()
    {
        for (const ReloadChange& change : m_registry.TakeChanges())
        {
            const Noesis::Uri uri(change.m_uri.c_str());
            if (change.m_kind == ReloadKind::Xaml)
            {
                m_xaml.RaiseXamlChanged(uri);
            }
            else
            {
                m_textures.RaiseTextureChanged(uri);
            }
        }
    }
}
