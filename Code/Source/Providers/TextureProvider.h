#pragma once

#include <NsGui/TextureProvider.h>

#include <Providers/ReloadRegistry.h>

namespace NoesisGUI
{
    class TextureProvider final : public Noesis::TextureProvider
    {
    public:
        explicit TextureProvider(ReloadRegistry* registry);

        Noesis::TextureInfo GetTextureInfo(const Noesis::Uri& uri) override;
        Noesis::Ptr<Noesis::Texture> LoadTexture(const Noesis::Uri& uri, Noesis::RenderDevice* device) override;

    private:
        ReloadRegistry* m_registry;
    };
}
