#pragma once

#include <Atom/RHI.Reflect/Format.h>
#include <AzCore/std/containers/span.h>

namespace NoesisGUI
{
    bool IsPremultipliableFormat(AZ::RHI::Format format);
    void PremultiplyRgba8(AZStd::span<AZ::u8> pixels);
}
