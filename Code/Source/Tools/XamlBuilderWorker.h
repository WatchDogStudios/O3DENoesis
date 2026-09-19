#pragma once

#include <AssetBuilderSDK/AssetBuilderSDK.h>

namespace NoesisGUI
{
    class XamlBuilderWorker
    {
    public:
        static constexpr const char* BusId = "{6A3C9E12-7B4D-4F28-9E61-2D8C5B0A3F17}";
        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const;
        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const;
    };
}
