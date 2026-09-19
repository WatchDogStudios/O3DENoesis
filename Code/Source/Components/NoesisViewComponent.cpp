#include <Components/NoesisViewComponent.h>

#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Render/NoesisFeatureProcessor.h>
#include <Runtime/EngineHooks.h>

#include <NsGui/FrameworkElement.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/IView.h>
#include <NsGui/Uri.h>

namespace NoesisGUI
{
    NoesisViewComponent::NoesisViewComponent() = default;
    NoesisViewComponent::~NoesisViewComponent() = default;

    void NoesisViewComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisViewComponent, AZ::Component>()
                ->Version(2)
                ->Field("Xaml", &NoesisViewComponent::m_xaml)
                ->Field("ZOrder", &NoesisViewComponent::m_zOrder)
                ->Field("PPAA", &NoesisViewComponent::m_ppaa)
                ->Field("Visible", &NoesisViewComponent::m_visible)
                ->Field("InputEnabled", &NoesisViewComponent::m_inputEnabled);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<NoesisViewComponent>("NoesisGUI View", "Renders a XAML user interface on screen.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "UI")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &NoesisViewComponent::m_xaml, "XAML", "Root XAML file of this view.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &NoesisViewComponent::m_zOrder, "Z order", "Views with higher values draw on top.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &NoesisViewComponent::m_ppaa, "PPAA", "Per-primitive antialiasing.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &NoesisViewComponent::m_visible, "Visible", "Draw this view.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &NoesisViewComponent::m_inputEnabled, "Input enabled", "Route mouse, touch, keyboard and gamepad input to this view.");
            }
        }
    }

    void NoesisViewComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NoesisViewService"));
    }

    Noesis::IView* NoesisViewComponent::GetView() const
    {
        return m_view.GetPtr();
    }

    Noesis::FrameworkElement* NoesisViewComponent::GetContent() const
    {
        return m_view ? m_view->GetContent() : nullptr;
    }

    void NoesisViewComponent::Activate()
    {
        m_featureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<NoesisFeatureProcessor>(GetEntityId());
        if (!m_featureProcessor || !m_xaml.GetId().IsValid())
        {
            AZ_Warning("NoesisGUI", m_featureProcessor, "NoesisGUI View on entity %s: no render scene.", GetEntityId().ToString().c_str());
            return;
        }

        // Noesis URIs name source files so relative references inside the XAML resolve against its source folder.
        AZStd::string uri = EngineHooks::GetAssetPath(m_xaml.GetId());
        const AZStd::string suffix = AZStd::string::format(".%s", NoesisXamlAsset::Extension);
        if (uri.ends_with(suffix))
        {
            uri.erase(uri.size() - suffix.size());
        }

        Noesis::Ptr<Noesis::FrameworkElement> content = Noesis::GUI::LoadXaml<Noesis::FrameworkElement>(Noesis::Uri(uri.c_str()));
        if (!content)
        {
            AZ_Error("NoesisGUI", false, "Failed to load XAML '%s'", uri.c_str());
            return;
        }
        m_view = Noesis::GUI::CreateView(content);
        m_view->SetFlags(m_ppaa ? Noesis::RenderFlags_PPAA : 0);
        m_featureProcessor->AddView(m_view.GetPtr(), m_zOrder, m_visible, m_inputEnabled);
    }

    void NoesisViewComponent::Deactivate()
    {
        if (m_view && m_featureProcessor)
        {
            m_featureProcessor->RemoveView(m_view.GetPtr());
        }
        m_view.Reset();
        m_featureProcessor = nullptr;
    }

    void NoesisViewComponent::SetVisible(bool visible)
    {
        m_visible = visible;
        if (m_view && m_featureProcessor)
        {
            m_featureProcessor->SetViewVisible(m_view.GetPtr(), visible);
        }
    }

    void NoesisViewComponent::SetInputEnabled(bool inputEnabled)
    {
        m_inputEnabled = inputEnabled;
        if (m_view && m_featureProcessor)
        {
            m_featureProcessor->SetViewInputEnabled(m_view.GetPtr(), inputEnabled);
        }
    }
}
