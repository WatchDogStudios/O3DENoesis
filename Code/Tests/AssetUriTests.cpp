#include <AzTest/AzTest.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <Providers/AssetStream.h>
#include <Providers/AssetUri.h>

namespace NoesisGUI::Test
{
    class AssetUriTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(AssetUriTests, ToProductPath_NormalizesAndAppendsExtension)
    {
        EXPECT_STREQ(AssetUri::ToProductPath("/UI\\Fonts/OpenSans.ttf", "noesisfont").c_str(), "ui/fonts/opensans.ttf.noesisfont");
        EXPECT_STREQ(AssetUri::ToProductPath("ui/Main.xaml", "noesisxaml").c_str(), "ui/main.xaml.noesisxaml");
        EXPECT_STREQ(AssetUri::ToProductPath("Images/Logo.PNG", "streamingimage").c_str(), "images/logo.png.streamingimage");
        EXPECT_STREQ(AssetUri::ToProductPath("UI/Fonts/", "").c_str(), "ui/fonts");
    }

    TEST_F(AssetUriTests, FolderAndSourceFileName)
    {
        EXPECT_STREQ(AssetUri::FolderOf("ui/fonts/a.ttf.noesisfont").c_str(), "ui/fonts");
        EXPECT_STREQ(AssetUri::FolderOf("a.ttf.noesisfont").c_str(), "");
        EXPECT_STREQ(AssetUri::SourceFileName("ui/fonts/a.ttf.noesisfont", "noesisfont").c_str(), "a.ttf");
    }

    TEST_F(AssetUriTests, ToProductPath_NormalizesTrailingSlashFolderBeforeJoining)
    {
        // Pins FontProvider::OpenFont's two-step normalization: the folder must be normalized on
        // its own before joining with the filename, or a trailing slash produces a double slash.
        const AZStd::string joined = AssetUri::ToProductPath("UI/Fonts/", {}) + "/" + "A.ttf";
        EXPECT_STREQ(AssetUri::ToProductPath(joined, "noesisfont").c_str(), "ui/fonts/a.ttf.noesisfont");
    }

    TEST_F(AssetUriTests, AssetStream_ReadSeekLength)
    {
        const AZStd::vector<AZ::u8> bytes = { 1, 2, 3, 4, 5 };
        Noesis::Ptr<AssetStream> stream = *new AssetStream({}, bytes);
        EXPECT_EQ(stream->GetLength(), 5u);
        AZ::u8 buffer[4] = {};
        EXPECT_EQ(stream->Read(buffer, 3), 3u);
        EXPECT_EQ(buffer[2], 3);
        EXPECT_EQ(stream->GetPosition(), 3u);
        EXPECT_EQ(stream->Read(buffer, 4), 2u);
        stream->SetPosition(1);
        EXPECT_EQ(static_cast<const AZ::u8*>(stream->GetMemoryBase())[stream->GetPosition()], 2);
    }
}
