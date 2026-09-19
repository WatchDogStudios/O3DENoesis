#include <Runtime/NoesisGUISystemComponent.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/parallel/mutex.h>
#include <NoesisGUI/NoesisGUITypeIds.h>
#include <Render/NoesisFeatureProcessor.h>
#include <Render/NoesisPass.h>
#include <Runtime/EngineHooks.h>
#include <Runtime/NoesisRuntime.h>

#include <NsGui/IntegrationAPI.h>
#include <NsGui/UIElement.h>

namespace NoesisGUI
{
    AZ_COMPONENT_IMPL(NoesisGUISystemComponent, "NoesisGUISystemComponent", NoesisGUISystemComponentTypeId);

    void NoesisGUISystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisGUISystemComponent, AZ::Component>()->Version(0);
        }

        NoesisFeatureProcessor::Reflect(context);
        NoesisXamlAsset::Reflect(context);
        NoesisFontAsset::Reflect(context);
    }

    void NoesisGUISystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NoesisGUIService"));
    }

    void NoesisGUISystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NoesisGUIService"));
    }

    void NoesisGUISystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("RPISystem"));
    }

    void NoesisGUISystemComponent::Activate()
    {
        NoesisRuntime::Acquire();

        m_xamlHandler = AZStd::make_unique<AzFramework::GenericAssetHandler<NoesisXamlAsset>>(
            NoesisXamlAsset::DisplayName, NoesisXamlAsset::Group, NoesisXamlAsset::Extension);
        m_xamlHandler->Register();
        m_fontHandler = AZStd::make_unique<AzFramework::GenericAssetHandler<NoesisFontAsset>>(
            NoesisFontAsset::DisplayName, NoesisFontAsset::Group, NoesisFontAsset::Extension);
        m_fontHandler->Register();

        m_xamlProvider = *new XamlProvider(&m_reloadRegistry);
        m_textureProvider = *new TextureProvider(&m_reloadRegistry);
        m_fontProvider = *new FontProvider();
        Noesis::GUI::SetXamlProvider(m_xamlProvider);
        Noesis::GUI::SetTextureProvider(m_textureProvider);
        Noesis::GUI::SetFontProvider(m_fontProvider);

        m_hotReload = AZStd::make_unique<HotReload>(m_reloadRegistry, *m_xamlProvider, *m_textureProvider);
        AZ::Interface<HotReload>::Register(m_hotReload.get());
        // ponytail: cursor shapes are not applied; O3DE 26.05 exposes no cursor-shape API (InputSystemCursorRequestBus sets visibility only).
        Noesis::GUI::SetSoftwareKeyboardCallback(nullptr,
            [](void*, Noesis::UIElement*, bool open)
            {
                EngineHooks::SetTextEntryActive(open);
            });

        if (auto* passSystem = AZ::RPI::PassSystemInterface::Get())
        {
            passSystem->AddPassCreator(AZ::Name("NoesisPass"), &NoesisPass::Create);
            m_loadTemplatesHandler = AZ::RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler(
                []()
                {
                    AZ::RPI::PassSystemInterface::Get()->LoadPassTemplateMappings("Passes/NoesisGUI/NoesisPassTemplates.azasset");
                });
            passSystem->ConnectEvent(m_loadTemplatesHandler);
        }
        if (auto* factory = AZ::RPI::FeatureProcessorFactory::Get())
        {
            factory->RegisterFeatureProcessor<NoesisFeatureProcessor>();
        }
    }

    void NoesisGUISystemComponent::Deactivate()
    {
        if (auto* factory = AZ::RPI::FeatureProcessorFactory::Get())
        {
            factory->UnregisterFeatureProcessor<NoesisFeatureProcessor>();
        }
        m_loadTemplatesHandler.Disconnect();

        {
            // NoesisPass::FrameBeginInternal reads AZ::Interface<HotReload>::Get() and calls Flush() on the render
            // thread under this same mutex; hold it here too so teardown can't race a concurrent Flush().
            AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
            AZ::Interface<HotReload>::Unregister(m_hotReload.get());
            m_hotReload.reset();
        }

        Noesis::GUI::SetSoftwareKeyboardCallback(nullptr, nullptr);
        Noesis::GUI::SetXamlProvider(nullptr);
        Noesis::GUI::SetTextureProvider(nullptr);
        Noesis::GUI::SetFontProvider(nullptr);
        m_fontProvider.Reset();
        m_textureProvider.Reset();
        m_xamlProvider.Reset();

        m_fontHandler->Unregister();
        m_fontHandler.reset();
        m_xamlHandler->Unregister();
        m_xamlHandler.reset();

        NoesisRuntime::Release();
    }
}
