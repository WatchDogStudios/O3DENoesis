#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/containers/vector.h>
#include <AzTest/AzTest.h>
#include <Providers/Premultiply.h>

namespace NoesisGUI::Test
{
    class PremultiplyTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(PremultiplyTests, ScalesColorByAlphaAndKeepsAlpha)
    {
        AZStd::vector<AZ::u8> pixels = {
            200, 100, 50, 255,   // opaque: unchanged
            200, 100, 50, 0,     // transparent: color zeroed
            200, 100, 50, 128,   // half: 200*128/255 = 100, 100*128/255 = 50, 50*128/255 = 25
        };
        PremultiplyRgba8(pixels);
        EXPECT_EQ(pixels, AZStd::vector<AZ::u8>({ 200, 100, 50, 255, 0, 0, 0, 0, 100, 50, 25, 128 }));
    }

    TEST_F(PremultiplyTests, IgnoresTrailingPartialPixel)
    {
        AZStd::vector<AZ::u8> pixels = { 10, 20, 30, 0, 99, 99 };
        PremultiplyRgba8(pixels);
        EXPECT_EQ(pixels, AZStd::vector<AZ::u8>({ 0, 0, 0, 0, 99, 99 }));
    }

    TEST_F(PremultiplyTests, OnlyUncompressedRgba8IsPremultipliable)
    {
        EXPECT_TRUE(IsPremultipliableFormat(AZ::RHI::Format::R8G8B8A8_UNORM));
        EXPECT_TRUE(IsPremultipliableFormat(AZ::RHI::Format::R8G8B8A8_UNORM_SRGB));
        EXPECT_FALSE(IsPremultipliableFormat(AZ::RHI::Format::BC1_UNORM_SRGB));
        EXPECT_FALSE(IsPremultipliableFormat(AZ::RHI::Format::R8_UNORM));
    }
}
