#pragma once

#include <AzCore/Outcome/Outcome.h>
#include <AzCore/std/containers/span.h>
#include <AzCore/std/string/string.h>

namespace NoesisGUI::XamlValidator
{
    //! ponytail: XML well-formedness only; Noesis-specific errors (unknown types, bad property values) still surface
    //! at load. Upgrade to semantic validation if malformed-but-well-formed XAML becomes a recurring failure mode.
    AZ::Outcome<void, AZStd::string> Validate(AZStd::span<const AZ::u8> xaml);
}
