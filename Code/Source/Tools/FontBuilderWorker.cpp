#include <Tools/FontBuilderWorker.h>

#include <AzCore/IO/Path/Path.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/Utils/Utils.h>
#include <NoesisGUI/NoesisFontAsset.h>

namespace NoesisGUI
{
    void FontBuilderWorker::CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const
    {
        for (const AssetBuilderSDK::PlatformInfo& platform : request.m_enabledPlatforms)
        {
            AssetBuilderSDK::JobDescriptor job;
            job.m_jobKey = "Noesis Font";
            job.SetPlatformIdentifier(platform.m_identifier.c_str());
            response.m_createJobOutputs.push_back(job);
        }
        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void FontBuilderWorker::ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        auto file = AZ::Utils::ReadFile<AZStd::vector<AZ::u8>>(request.m_fullPath);
        if (!file.IsSuccess())
        {
            AZ_Error("NoesisGUI", false, "Cannot read %s: %s", request.m_fullPath.c_str(), file.GetError().c_str());
            return;
        }

        NoesisFontAsset asset;
        asset.m_data = file.TakeValue();
        AZ::IO::Path productPath = AZ::IO::Path(request.m_tempDirPath) / AZ::IO::PathView(request.m_sourceFile).Filename();
        productPath.Native() += AZStd::string::format(".%s", NoesisFontAsset::Extension);
        if (!AZ::Utils::SaveObjectToFile(productPath.Native(), AZ::DataStream::ST_BINARY, &asset))
        {
            AZ_Error("NoesisGUI", false, "Cannot write %s", productPath.c_str());
            return;
        }

        AssetBuilderSDK::JobProduct product(productPath.Native(), azrtti_typeid<NoesisFontAsset>(), 0);
        product.m_dependenciesHandled = true;
        response.m_outputProducts.push_back(AZStd::move(product));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}
