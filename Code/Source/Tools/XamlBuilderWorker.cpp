#include <Tools/XamlBuilderWorker.h>

#include <AzCore/IO/Path/Path.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/Utils/Utils.h>
#include <NoesisGUI/NoesisXamlAsset.h>
#include <Tools/XamlDependencyScanner.h>
#include <Tools/XamlValidator.h>

namespace NoesisGUI
{
    void XamlBuilderWorker::CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const
    {
        for (const AssetBuilderSDK::PlatformInfo& platform : request.m_enabledPlatforms)
        {
            AssetBuilderSDK::JobDescriptor job;
            job.m_jobKey = "Noesis XAML";
            job.SetPlatformIdentifier(platform.m_identifier.c_str());
            response.m_createJobOutputs.push_back(job);
        }
        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void XamlBuilderWorker::ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        auto file = AZ::Utils::ReadFile<AZStd::vector<AZ::u8>>(request.m_fullPath);
        if (!file.IsSuccess())
        {
            AZ_Error("NoesisGUI", false, "Cannot read %s: %s", request.m_fullPath.c_str(), file.GetError().c_str());
            return;
        }

        NoesisXamlAsset asset;
        asset.m_data = file.TakeValue();

        if (const auto valid = XamlValidator::Validate(asset.m_data); !valid.IsSuccess())
        {
            AZ_Error("NoesisGUI", false, "%s: %s", request.m_sourceFile.c_str(), valid.GetError().c_str());
            return;
        }

        const AZStd::string sourceRelativePath = AZ::IO::Path(request.m_sourceFile).AsPosix();
        const auto dependencies = XamlDependencyScanner::Scan(asset.m_data, sourceRelativePath);

        AZ::IO::Path productPath = AZ::IO::Path(request.m_tempDirPath) / AZ::IO::PathView(request.m_sourceFile).Filename();
        productPath.Native() += AZStd::string::format(".%s", NoesisXamlAsset::Extension);
        if (!AZ::Utils::SaveObjectToFile(productPath.Native(), AZ::DataStream::ST_BINARY, &asset))
        {
            AZ_Error("NoesisGUI", false, "Cannot write %s", productPath.c_str());
            return;
        }

        AssetBuilderSDK::JobProduct product(productPath.Native(), azrtti_typeid<NoesisXamlAsset>(), 0);
        for (const XamlDependencyScanner::Dependency& dependency : dependencies)
        {
            if (dependency.m_isFontFolder)
            {
                for (const char* pattern : { "/*.ttf", "/*.otf", "/*.ttc" })
                {
                    product.m_pathDependencies.emplace(dependency.m_path + pattern, AssetBuilderSDK::ProductPathDependencyType::SourceFile);
                }
            }
            else
            {
                product.m_pathDependencies.emplace(dependency.m_path, AssetBuilderSDK::ProductPathDependencyType::SourceFile);
            }
        }
        product.m_dependenciesHandled = true;
        response.m_outputProducts.push_back(AZStd::move(product));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}
