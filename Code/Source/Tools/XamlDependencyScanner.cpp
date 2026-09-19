#include <Tools/XamlDependencyScanner.h>

#include <NsCore/String.h>
#include <NsGui/Enums.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/MemoryStream.h>
#include <NsGui/Uri.h>

namespace NoesisGUI::XamlDependencyScanner
{
    AZStd::vector<Dependency> Scan(AZStd::span<const AZ::u8> xaml, const AZStd::string& sourceRelativePath)
    {
        AZStd::vector<Dependency> result;
        Noesis::Ptr<Noesis::MemoryStream> stream = *new Noesis::MemoryStream(xaml.data(), static_cast<uint32_t>(xaml.size()));
        Noesis::GUI::GetXamlDependencies(stream, Noesis::Uri(sourceRelativePath.c_str()), &result,
            [](void* user, const Noesis::Uri& uri, Noesis::XamlDependencyType type)
            {
                // Only Filename/Font carry real asset paths; Root/UserControl/Class report reflection type info.
                if (type != Noesis::XamlDependencyType_Filename && type != Noesis::XamlDependencyType_Font)
                {
                    return;
                }
                Noesis::FixedString<512> uriPath;
                uri.GetPath(uriPath);
                AZStd::string path(uriPath.Str());
                const bool isFont = type == Noesis::XamlDependencyType_Font;
                if (isFont)
                {
                    // Font URIs are "<folder>/#<Family>"; the dependency is the folder.
                    path = path.substr(0, path.find('#'));
                }
                while (!path.empty() && (path.front() == '/' || path.front() == '\\'))
                {
                    path.erase(path.begin());
                }
                while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
                {
                    path.pop_back();
                }
                if (!path.empty())
                {
                    static_cast<AZStd::vector<Dependency>*>(user)->push_back({ path, isFont });
                }
            });
        return result;
    }
}
