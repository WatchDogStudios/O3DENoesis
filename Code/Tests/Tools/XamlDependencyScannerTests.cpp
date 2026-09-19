#include <AzCore/UnitTest/TestTypes.h>
#include <AzTest/AzTest.h>
#include <Tools/XamlDependencyScanner.h>

namespace NoesisGUI::Test
{
    class XamlDependencyScannerTests : public UnitTest::LeakDetectionFixture
    {
    protected:
        AZStd::vector<XamlDependencyScanner::Dependency> Scan(const char* xaml, const char* path)
        {
            const AZStd::string_view text(xaml);
            return XamlDependencyScanner::Scan(
                AZStd::span<const AZ::u8>(reinterpret_cast<const AZ::u8*>(text.data()), text.size()), path);
        }

        static bool Contains(const AZStd::vector<XamlDependencyScanner::Dependency>& deps, const char* path, bool fontFolder)
        {
            for (const auto& dep : deps)
            {
                if (dep.m_path == path && dep.m_isFontFolder == fontFolder)
                {
                    return true;
                }
            }
            return false;
        }
    };

    TEST_F(XamlDependencyScannerTests, RelativeImageResourceAndFont)
    {
        const char* xaml = R"(
<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
      xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
  <Grid.Resources>
    <ResourceDictionary Source="Styles/Buttons.xaml"/>
  </Grid.Resources>
  <Image Source="Images/logo.png"/>
  <TextBlock FontFamily="Fonts/#Open Sans" Text="Hi"/>
</Grid>)";
        const auto deps = Scan(xaml, "ui/Main.xaml");
        EXPECT_TRUE(Contains(deps, "ui/Styles/Buttons.xaml", false));
        EXPECT_TRUE(Contains(deps, "ui/Images/logo.png", false));
        EXPECT_TRUE(Contains(deps, "ui/Fonts", true));
    }

    TEST_F(XamlDependencyScannerTests, PackUriDropsAssembly)
    {
        const char* xaml = R"(
<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
  <Image Source="/Game;component/Shared/icon.png"/>
</Grid>)";
        const auto deps = Scan(xaml, "ui/Main.xaml");
        EXPECT_TRUE(Contains(deps, "Shared/icon.png", false));
    }

    TEST_F(XamlDependencyScannerTests, NoDependencies)
    {
        EXPECT_TRUE(Scan(R"(<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"/>)", "a.xaml").empty());
    }
}
