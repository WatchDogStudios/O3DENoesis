#include <AzCore/UnitTest/TestTypes.h>
#include <AzTest/AzTest.h>
#include <Providers/ReloadRegistry.h>

namespace NoesisGUI::Test
{
    class ReloadRegistryTests : public UnitTest::LeakDetectionFixture
    {
    protected:
        const AZ::Data::AssetId m_xaml{ AZ::Uuid("{11111111-1111-1111-1111-111111111111}"), 0 };
        const AZ::Data::AssetId m_image{ AZ::Uuid("{22222222-2222-2222-2222-222222222222}"), 0 };
    };

    TEST_F(ReloadRegistryTests, RecordedChangeIsReturnedOnce)
    {
        ReloadRegistry registry;
        registry.Record(m_xaml, ReloadKind::Xaml, "ui/Main.xaml");
        registry.MarkChanged(m_xaml);
        registry.MarkChanged(m_xaml);

        const auto changes = registry.TakeChanges();
        ASSERT_EQ(changes.size(), 1u);
        EXPECT_EQ(changes[0].m_kind, ReloadKind::Xaml);
        EXPECT_STREQ(changes[0].m_uri.c_str(), "ui/Main.xaml");
        EXPECT_TRUE(registry.TakeChanges().empty());
    }

    TEST_F(ReloadRegistryTests, UnrecordedAssetsAreIgnored)
    {
        ReloadRegistry registry;
        registry.MarkChanged(m_image);
        EXPECT_TRUE(registry.TakeChanges().empty());
    }

    TEST_F(ReloadRegistryTests, OneAssetCanServeSeveralUris)
    {
        ReloadRegistry registry;
        registry.Record(m_image, ReloadKind::Texture, "ui/logo.png");
        registry.Record(m_image, ReloadKind::Texture, "UI/Logo.png");
        registry.Record(m_image, ReloadKind::Texture, "ui/logo.png");
        registry.MarkChanged(m_image);
        EXPECT_EQ(registry.TakeChanges().size(), 2u);
    }

    TEST_F(ReloadRegistryTests, RecordsSurviveAcrossChanges)
    {
        ReloadRegistry registry;
        registry.Record(m_xaml, ReloadKind::Xaml, "a.xaml");
        registry.MarkChanged(m_xaml);
        registry.TakeChanges();
        registry.MarkChanged(m_xaml);
        EXPECT_EQ(registry.TakeChanges().size(), 1u);
    }
}
