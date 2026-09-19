#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/std/string/string_view.h>

namespace NoesisGUI::AssetUri
{
    AZStd::string ToProductPath(AZStd::string_view uriPath, AZStd::string_view productExtension);
    AZStd::string FolderOf(AZStd::string_view productPath);
    AZStd::string SourceFileName(AZStd::string_view productPath, AZStd::string_view productExtension);
}
