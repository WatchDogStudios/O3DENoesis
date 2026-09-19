#pragma once

#include <AzCore/std/containers/span.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

namespace NoesisGUI::XamlDependencyScanner
{
    struct Dependency
    {
        AZStd::string m_path;
        bool m_isFontFolder = false;
    };

    AZStd::vector<Dependency> Scan(AZStd::span<const AZ::u8> xaml, const AZStd::string& sourceRelativePath);
}
