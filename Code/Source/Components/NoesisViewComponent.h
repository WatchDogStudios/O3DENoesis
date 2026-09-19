#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>
#include <NoesisGUI/NoesisXamlAsset.h>

#include <NsCore/Ptr.h>

namespace Noesis
{
    class FrameworkElement;
    struct IView;
}

namespace NoesisGUI
{
    class NoesisFeatureProcessor;

    class NoesisViewComponent final : public AZ::Component
    {
    public:
        AZ_COMPONENT(NoesisViewComponent, "{B8E3D5A1-4C7F-4E92-8A16-3D9F0C2B7E55}");

        NoesisViewComponent();
        ~NoesisViewComponent() override;

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        Noesis::IView* GetView() const;
        Noesis::FrameworkElement* GetContent() const;

        void SetVisible(bool visible);
        void SetInputEnabled(bool inputEnabled);

    private:
        void Activate() override;
        void Deactivate() override;

        AZ::Data::Asset<NoesisXamlAsset> m_xaml{ AZ::Data::AssetLoadBehavior::NoLoad };
        AZ::s32 m_zOrder = 0;
        bool m_ppaa = true;
        bool m_visible = true;
        bool m_inputEnabled = true;

        Noesis::Ptr<Noesis::IView> m_view;
        NoesisFeatureProcessor* m_featureProcessor = nullptr;
    };
}
