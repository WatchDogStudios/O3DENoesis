#pragma once

#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <AzCore/Component/Component.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzFramework/Asset/GenericAssetHandler.h>
#include <NoesisGUI/NoesisFontAsset.h>
#include <NoesisGUI/NoesisXamlAsset.h>
#include <Providers/FontProvider.h>
#include <Providers/HotReload.h>
#include <Providers/ReloadRegistry.h>
#include <Providers/TextureProvider.h>
#include <Providers/XamlProvider.h>

#include <NsCore/Ptr.h>

namespace NoesisGUI
{
    class NoesisGUISystemComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT_DECL(NoesisGUISystemComponent);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        AZ::RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler m_loadTemplatesHandler;
        AZStd::unique_ptr<AzFramework::GenericAssetHandler<NoesisXamlAsset>> m_xamlHandler;
        AZStd::unique_ptr<AzFramework::GenericAssetHandler<NoesisFontAsset>> m_fontHandler;

        ReloadRegistry m_reloadRegistry;
        AZStd::unique_ptr<HotReload> m_hotReload;

        Noesis::Ptr<XamlProvider> m_xamlProvider;
        Noesis::Ptr<TextureProvider> m_textureProvider;
        Noesis::Ptr<FontProvider> m_fontProvider;
    };
}
