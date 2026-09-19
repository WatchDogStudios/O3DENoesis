#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/string/string_view.h>
#include <AzTest/AzTest.h>
#include <Tools/XamlValidator.h>

namespace NoesisGUI::Test
{
    class XamlValidatorTests : public UnitTest::LeakDetectionFixture
    {
    protected:
        static AZ::Outcome<void, AZStd::string> Validate(AZStd::string_view text)
        {
            return XamlValidator::Validate(AZStd::span<const AZ::u8>(reinterpret_cast<const AZ::u8*>(text.data()), text.size()));
        }
    };

    TEST_F(XamlValidatorTests, WellFormedXamlPasses)
    {
        EXPECT_TRUE(Validate(R"(<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"><TextBlock Text="Hi"/></Grid>)").IsSuccess());
    }

    TEST_F(XamlValidatorTests, UnclosedElementFailsWithMessage)
    {
        const auto outcome = Validate(R"(<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"><TextBlock Text="Hi"></Grid>)");
        ASSERT_FALSE(outcome.IsSuccess());
        EXPECT_FALSE(outcome.GetError().empty());
    }

    TEST_F(XamlValidatorTests, EmptyInputFails)
    {
        EXPECT_FALSE(Validate("").IsSuccess());
    }

    TEST_F(XamlValidatorTests, NoRootElementFails)
    {
        EXPECT_FALSE(Validate("   \n\t  ").IsSuccess());
    }

    TEST_F(XamlValidatorTests, Utf16LeBomPasses)
    {
        // BOM (0xFF 0xFE) + "<G/>" as UTF-16LE; the validator early-outs on the BOM without parsing.
        const AZ::u8 utf16[] = { 0xFF, 0xFE, 0x3C, 0x00, 0x47, 0x00, 0x2F, 0x00, 0x3E, 0x00 };
        EXPECT_TRUE(XamlValidator::Validate(AZStd::span<const AZ::u8>(utf16, sizeof(utf16))).IsSuccess());
    }
}
