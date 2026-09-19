#include <AzTest/AzTest.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <Render/AtomTexture.h>

namespace NoesisGUI::Test
{
    class AtomTextureTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(AtomTextureTests, Properties)
    {
        Noesis::Ptr<AtomTexture> texture = *new AtomTexture({}, 64, 32, 3, false);
        EXPECT_EQ(texture->GetWidth(), 64u);
        EXPECT_EQ(texture->GetHeight(), 32u);
        EXPECT_TRUE(texture->HasMipMaps());
        EXPECT_FALSE(texture->IsInverted());
        EXPECT_FALSE(texture->HasAlpha());
        EXPECT_FALSE(texture->IsRenderTarget());
        texture->MarkRenderTarget();
        EXPECT_TRUE(texture->IsRenderTarget());
        EXPECT_EQ(texture->GetAttachmentImage(), nullptr);
    }

    TEST_F(AtomTextureTests, IdsAreUniqueAndIncreasing)
    {
        Noesis::Ptr<AtomTexture> a = *new AtomTexture({}, 1, 1, 1, true);
        Noesis::Ptr<AtomTexture> b = *new AtomTexture({}, 1, 1, 1, true);
        EXPECT_LT(a->GetId(), b->GetId());
    }

    TEST_F(AtomTextureTests, RenderTargetExposesColor)
    {
        Noesis::Ptr<AtomTexture> color = *new AtomTexture({}, 8, 8, 1, true);
        color->MarkRenderTarget();
        Noesis::Ptr<AtomRenderTarget> target = *new AtomRenderTarget(color, {});
        EXPECT_EQ(target->GetTexture(), color.GetPtr());
        EXPECT_EQ(target->GetStencil(), nullptr);
    }
}
