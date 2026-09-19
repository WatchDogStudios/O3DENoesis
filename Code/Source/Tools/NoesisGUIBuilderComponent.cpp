#include <Tools/NoesisGUIBuilderComponent.h>

#include <AssetBuilderSDK/AssetBuilderSDK.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <Runtime/NoesisRuntime.h>

namespace NoesisGUI
{
    void NoesisGUIBuilderComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisGUIBuilderComponent, AZ::Component>()
                ->Version(1)
                ->Attribute(AZ::Edit::Attributes::SystemComponentTags, AZStd::vector<AZ::Crc32>({ AssetBuilderSDK::ComponentTags::AssetBuilder }));
        }
    }

    void NoesisGUIBuilderComponent::Activate()
    {
        NoesisRuntime::Acquire();

        auto registerBuilder = [this](const char* name, const char* busId, AZStd::initializer_list<const char*> patterns,
                                      AZStd::function<void(const AssetBuilderSDK::CreateJobsRequest&, AssetBuilderSDK::CreateJobsResponse&)> createJobs,
                                      AZStd::function<void(const AssetBuilderSDK::ProcessJobRequest&, AssetBuilderSDK::ProcessJobResponse&)> processJob)
        {
            AssetBuilderSDK::AssetBuilderDesc desc;
            desc.m_name = name;
            for (const char* pattern : patterns)
            {
                desc.m_patterns.emplace_back(pattern, AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard);
            }
            desc.m_busId = AZ::Uuid(busId);
            desc.m_version = 1;
            desc.m_createJobFunction = AZStd::move(createJobs);
            desc.m_processJobFunction = AZStd::move(processJob);
            BusConnect(desc.m_busId);
            AssetBuilderSDK::AssetBuilderBus::Broadcast(&AssetBuilderSDK::AssetBuilderBusTraits::RegisterBuilderInformation, desc);
        };

        registerBuilder("Noesis XAML Builder", XamlBuilderWorker::BusId, { "*.xaml" },
            [this](const auto& request, auto& response) { m_xamlWorker.CreateJobs(request, response); },
            [this](const auto& request, auto& response) { m_xamlWorker.ProcessJob(request, response); });
        registerBuilder("Noesis Font Builder", FontBuilderWorker::BusId, { "*.ttf", "*.otf", "*.ttc" },
            [this](const auto& request, auto& response) { m_fontWorker.CreateJobs(request, response); },
            [this](const auto& request, auto& response) { m_fontWorker.ProcessJob(request, response); });
    }

    void NoesisGUIBuilderComponent::Deactivate()
    {
        BusDisconnect();
        NoesisRuntime::Release();
    }
}
