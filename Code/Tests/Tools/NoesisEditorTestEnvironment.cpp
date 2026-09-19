#include <AzCore/std/string/string_view.h>
#include <AzTest/AzTest.h>
#include <Runtime/NoesisRuntime.h>
#include <Tools/XamlDependencyScanner.h>

namespace NoesisGUI::Test
{
    class NoesisEditorEnvironment : public AZ::Test::ITestEnvironment
    {
    protected:
        void SetupEnvironment() override
        {
            NoesisRuntime::Acquire();

            // Force Noesis's lazily-initialized reflection caches (TypeClass::GetProperty etc.) to allocate here,
            // outside any test's leak-detection window, rather than on whichever test happens to touch a given XAML
            // element/property type first. Covers every element type XamlDependencyScannerTests exercises.
            const AZStd::string_view warmup = R"(
<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
      xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
  <Grid.Resources>
    <ResourceDictionary Source="Styles/Buttons.xaml"/>
  </Grid.Resources>
  <Image Source="Images/logo.png"/>
  <TextBlock FontFamily="Fonts/#Open Sans" Text="Hi"/>
</Grid>)";
            XamlDependencyScanner::Scan(
                AZStd::span<const AZ::u8>(reinterpret_cast<const AZ::u8*>(warmup.data()), warmup.size()), "warmup.xaml");
        }

        void TeardownEnvironment() override
        {
            NoesisRuntime::Release();
        }
    };
}

AZ_UNIT_TEST_HOOK(new NoesisGUI::Test::NoesisEditorEnvironment);
