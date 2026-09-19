#include <AzTest/AzTest.h>
#include <Render/ShaderTable.h>
#include <Runtime/NoesisRuntime.h>

#include <AzCore/Name/NameDictionary.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/string/string.h>

#include <NsCore/Version.h>
#include <NsGui/ItemsControl.h>
#include <NsGui/ObservableCollection.h>

namespace NoesisGUI::Test
{
    class NoesisEnvironment : public AZ::Test::ITestEnvironment
    {
    protected:
        void SetupEnvironment() override
        {
            // ShaderTable's InputStreamLayoutBuilder::Channel() parses AZ::Name semantics, which
            // requires the global NameDictionary; it is normally created by ComponentApplication.
            if (!AZ::NameDictionary::IsReady())
            {
                AZ::NameDictionary::Create();
            }
            NoesisRuntime::Acquire();

            // AZ::Name interning (eg. the vertex-semantic names ShaderTable::BuildInputStreamLayout parses)
            // is process-lifetime by design, so it must happen before any test's leak-detection baseline or
            // the first test to touch it gets blamed for a "leak" that's really a one-time global cache fill.
            for (int i = 0; i < Noesis::Shader::Count; ++i)
            {
                const auto shader = static_cast<Noesis::Shader::Enum>(i);
                if (ShaderTable::IsSupported(shader))
                {
                    ShaderTable::BuildInputStreamLayout(shader);
                }
            }

            // ObservableCollection<BaseComponent> and ItemsControl.ItemsSource build their reflection
            // property/converter caches lazily on first use, and those caches are process-lifetime too.
            Noesis::Ptr<Noesis::ItemsControl> warmItemsControl = *new Noesis::ItemsControl();
            Noesis::Ptr<Noesis::ObservableCollection<Noesis::BaseComponent>> warmItems =
                *new Noesis::ObservableCollection<Noesis::BaseComponent>();
            warmItemsControl->SetItemsSource(warmItems);
            warmItemsControl->GetItems();
        }

        void TeardownEnvironment() override
        {
            NoesisRuntime::Release();
            if (AZ::NameDictionary::IsReady())
            {
                AZ::NameDictionary::Destroy();
            }
        }
    };

    class NoesisRuntimeTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(NoesisRuntimeTests, BuildVersion_IsSdk4)
    {
        const AZStd::string version = Noesis::GetBuildVersion();
        EXPECT_TRUE(version.starts_with("4.")) << version.c_str();
    }

    TEST_F(NoesisRuntimeTests, AcquireRelease_IsRefcounted)
    {
        NoesisRuntime::Acquire();
        EXPECT_EQ(NoesisRuntime::GetRefCount(), 2u);
        NoesisRuntime::Release();
        EXPECT_EQ(NoesisRuntime::GetRefCount(), 1u);
    }
}

AZ_UNIT_TEST_HOOK(new NoesisGUI::Test::NoesisEnvironment);
