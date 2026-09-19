#include <Providers/AssetUri.h>

#include <AzCore/std/algorithm.h>
#include <AzCore/std/string/conversions.h>

namespace NoesisGUI::AssetUri
{
    AZStd::string ToProductPath(AZStd::string_view uriPath, AZStd::string_view productExtension)
    {
        AZStd::string path(uriPath);
        AZStd::replace(path.begin(), path.end(), '\\', '/');
        AZStd::to_lower(path.begin(), path.end());
        while (!path.empty() && path.front() == '/')
        {
            path.erase(path.begin());
        }
        while (!path.empty() && path.back() == '/')
        {
            path.pop_back();
        }
        if (!productExtension.empty())
        {
            path.append(".");
            path.append(productExtension);
        }
        return path;
    }

    AZStd::string FolderOf(AZStd::string_view productPath)
    {
        const size_t slash = productPath.rfind('/');
        return slash == AZStd::string_view::npos ? AZStd::string() : AZStd::string(productPath.substr(0, slash));
    }

    AZStd::string SourceFileName(AZStd::string_view productPath, AZStd::string_view productExtension)
    {
        const size_t slash = productPath.rfind('/');
        AZStd::string_view name = slash == AZStd::string_view::npos ? productPath : productPath.substr(slash + 1);
        if (name.size() > productExtension.size() + 1 && name.ends_with(productExtension))
        {
            name.remove_suffix(productExtension.size() + 1);
        }
        return AZStd::string(name);
    }
}
