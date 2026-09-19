#pragma once

#include <AssetBuilderSDK/AssetBuilderSDK.h>

namespace NoesisGUI
{
    class FontBuilderWorker
    {
    public:
        static constexpr const char* BusId = "{C1E47B85-3F2A-4D96-8B07-5A9E6D3C2B48}";
        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const;
        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const;
    };
}
