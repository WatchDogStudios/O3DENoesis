#include <AzTest/AzTest.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <Render/RenderTape.h>

namespace NoesisGUI::Test
{
    struct FakeTarget {};
    struct FakeTexture {};
    using Tape = RenderTape<int, FakeTarget, FakeTexture>;

    class RenderTapeTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(RenderTapeTests, OffscreenThenOnscreen_PreservesOrderAndRanges)
    {
        FakeTarget a, b;
        FakeTexture ta;
        Tape tape;
        tape.SetTarget(&a);
        tape.AddDraw(1, {});
        tape.AddDraw(2, {});
        tape.SetTarget(&b);
        const FakeTexture* sampled[] = { &ta };
        tape.AddDraw(3, sampled);
        tape.SetTarget(nullptr);
        tape.AddDraw(4, sampled);
        tape.AddDraw(5, sampled);

        ASSERT_EQ(tape.GetScopes().size(), 3u);
        EXPECT_EQ(tape.GetScopes()[0].m_target, &a);
        EXPECT_EQ(tape.GetScopes()[0].m_firstDraw, 0u);
        EXPECT_EQ(tape.GetScopes()[0].m_drawCount, 2u);
        EXPECT_EQ(tape.GetScopes()[1].m_firstDraw, 2u);
        EXPECT_EQ(tape.GetScopes()[2].m_target, nullptr);
        EXPECT_EQ(tape.GetScopes()[2].m_drawCount, 2u);
        EXPECT_EQ(tape.GetScopes()[2].m_sampledRenderTargets.size(), 1u);
        EXPECT_EQ(tape.GetOnscreenDrawCount(), 2u);
        EXPECT_EQ(tape.GetDraws()[4], 5);
    }

    TEST_F(RenderTapeTests, EmptyScopesDropped_MultipleOnscreenScopesSummed)
    {
        FakeTarget a;
        Tape tape;
        tape.SetTarget(&a);
        tape.SetTarget(nullptr);
        tape.AddDraw(1, {});
        tape.SetTarget(nullptr);
        tape.AddDraw(2, {});
        EXPECT_EQ(tape.GetScopes().size(), 2u);
        EXPECT_EQ(tape.GetOnscreenDrawCount(), 2u);
    }

    TEST_F(RenderTapeTests, TrailingEmptyScope_DroppedByGetScopes)
    {
        FakeTarget a;
        Tape tape;
        tape.SetTarget(nullptr);
        tape.AddDraw(1, {});
        tape.SetTarget(&a);

        ASSERT_EQ(tape.GetScopes().size(), 1u);
        EXPECT_EQ(tape.GetScopes()[0].m_target, nullptr);
        EXPECT_EQ(tape.GetDraws().size(), 1u);

        tape.SetTarget(&a);
        tape.AddDraw(2, {});
        EXPECT_EQ(tape.GetScopes().size(), 2u);
    }

    TEST_F(RenderTapeTests, Reset_ClearsEverything)
    {
        Tape tape;
        tape.SetTarget(nullptr);
        tape.AddDraw(1, {});
        tape.Reset();
        EXPECT_TRUE(tape.GetScopes().empty());
        EXPECT_TRUE(tape.GetDraws().empty());
    }
}
