#include <Providers/Premultiply.h>

namespace NoesisGUI
{
    bool IsPremultipliableFormat(AZ::RHI::Format format)
    {
        return format == AZ::RHI::Format::R8G8B8A8_UNORM || format == AZ::RHI::Format::R8G8B8A8_UNORM_SRGB;
    }

    void PremultiplyRgba8(AZStd::span<AZ::u8> pixels)
    {
        const size_t pixelBytes = pixels.size() - pixels.size() % 4;
        for (size_t i = 0; i < pixelBytes; i += 4)
        {
            const AZ::u32 alpha = pixels[i + 3];
            pixels[i] = static_cast<AZ::u8>(pixels[i] * alpha / 255);
            pixels[i + 1] = static_cast<AZ::u8>(pixels[i + 1] * alpha / 255);
            pixels[i + 2] = static_cast<AZ::u8>(pixels[i + 2] * alpha / 255);
        }
    }
}
