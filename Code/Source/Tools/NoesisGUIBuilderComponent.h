#pragma once

#include <AssetBuilderSDK/AssetBuilderBusses.h>
#include <AzCore/Component/Component.h>
#include <Tools/FontBuilderWorker.h>
#include <Tools/XamlBuilderWorker.h>

namespace NoesisGUI
{
    class NoesisGUIBuilderComponent final
        : public AZ::Component
        , private AssetBuilderSDK::AssetBuilderCommandBus::MultiHandler
    {
    public:
        AZ_COMPONENT(NoesisGUIBuilderComponent, "{E5B2A7C3-8D1F-4B69-9C3E-7F0A2D5B6E81}");

        static void Reflect(AZ::ReflectContext* context);

    private:
        void Activate() override;
        void Deactivate() override;
        void ShutDown() override {}

        XamlBuilderWorker m_xamlWorker;
        FontBuilderWorker m_fontWorker;
    };
}
