#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <Providers/ReloadRegistry.h>
#include <Runtime/EngineHooks.h>

namespace NoesisGUI
{
    class XamlProvider;
    class TextureProvider;

    class HotReload
    {
    public:
        AZ_RTTI(NoesisGUI::HotReload, "{8B3E5D17-2C4A-4F6E-9A81-5D0C7B2E9F43}");
        AZ_CLASS_ALLOCATOR(HotReload, AZ::SystemAllocator);

        HotReload(ReloadRegistry& registry, XamlProvider& xaml, TextureProvider& textures);
        virtual ~HotReload();

        void Flush();

    private:
        ReloadRegistry& m_registry;
        XamlProvider& m_xaml;
        TextureProvider& m_textures;
        AZStd::unique_ptr<EngineHooks::Connection> m_catalogConnection;
    };
}
