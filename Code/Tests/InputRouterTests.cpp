#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <AzTest/AzTest.h>
#include <Input/InputRouter.h>

namespace NoesisGUI::Test
{
    struct FakeTarget : InputTarget
    {
        bool m_handles = true;
        AZStd::vector<AZStd::string> m_calls;

        bool Record(const AZStd::string& call)
        {
            m_calls.push_back(call);
            return m_handles;
        }
        bool MouseMove(int x, int y) override { return Record(AZStd::string::format("move %d %d", x, y)); }
        bool MouseButtonDown(int x, int y, Noesis::MouseButton b) override { return Record(AZStd::string::format("down %d %d %d", x, y, b)); }
        bool MouseButtonUp(int x, int y, Noesis::MouseButton b) override { return Record(AZStd::string::format("up %d %d %d", x, y, b)); }
        bool MouseWheel(int x, int y, int r) override { return Record(AZStd::string::format("wheel %d %d %d", x, y, r)); }
        bool TouchDown(int x, int y, AZ::u64 id) override { AZ_UNUSED(x, y); return Record(AZStd::string::format("tdown %llu", static_cast<unsigned long long>(id))); }
        bool TouchMove(int x, int y, AZ::u64 id) override { AZ_UNUSED(x, y); return Record(AZStd::string::format("tmove %llu", static_cast<unsigned long long>(id))); }
        bool TouchUp(int x, int y, AZ::u64 id) override { AZ_UNUSED(x, y); return Record(AZStd::string::format("tup %llu", static_cast<unsigned long long>(id))); }
        bool KeyDown(Noesis::Key k) override { return Record(AZStd::string::format("kdown %d", k)); }
        bool KeyUp(Noesis::Key k) override { return Record(AZStd::string::format("kup %d", k)); }
        bool Char(AZ::u32 c) override { return Record(AZStd::string::format("char %u", c)); }
    };

    class InputRouterTests : public UnitTest::LeakDetectionFixture
    {
    protected:
        void SetUp() override
        {
            UnitTest::LeakDetectionFixture::SetUp();
            m_router.SetViewportSize(1000, 500);
        }

        InputRouter m_router;
        FakeTarget m_bottom;
        FakeTarget m_top;
    };

    TEST_F(InputRouterTests, MoveReachesAllTargetsInPixels)
    {
        InputTarget* targets[] = { &m_bottom, &m_top };
        EXPECT_TRUE(m_router.PointerMove(AZ::Vector2(0.5f, 0.2f), targets));
        ASSERT_EQ(m_bottom.m_calls.size(), 1u);
        EXPECT_EQ(m_bottom.m_calls[0], "move 500 100");
        EXPECT_EQ(m_top.m_calls[0], "move 500 100");
    }

    TEST_F(InputRouterTests, PressGoesTopDownUntilHandledAndSetsFocus)
    {
        m_top.m_handles = false;
        InputTarget* targets[] = { &m_bottom, &m_top };
        EXPECT_TRUE(m_router.MouseButton(AZ::Vector2(0.0f, 0.0f), Noesis::MouseButton_Left, true, targets));
        EXPECT_EQ(m_top.m_calls.size(), 1u);
        EXPECT_EQ(m_bottom.m_calls.size(), 1u);

        EXPECT_TRUE(m_router.Key(Noesis::Key_A, true, targets));
        EXPECT_EQ(m_bottom.m_calls.back(), AZStd::string::format("kdown %d", Noesis::Key_A));
        EXPECT_EQ(m_top.m_calls.size(), 1u);
    }

    TEST_F(InputRouterTests, CaptureReceivesMoveAndReleaseOnly)
    {
        InputTarget* targets[] = { &m_bottom, &m_top };
        m_router.MouseButton(AZ::Vector2(0.1f, 0.1f), Noesis::MouseButton_Left, true, targets);
        m_top.m_calls.clear();
        m_bottom.m_calls.clear();

        m_router.PointerMove(AZ::Vector2(0.2f, 0.2f), targets);
        m_router.MouseButton(AZ::Vector2(0.2f, 0.2f), Noesis::MouseButton_Left, false, targets);
        EXPECT_EQ(m_top.m_calls.size(), 2u);
        EXPECT_TRUE(m_bottom.m_calls.empty());

        m_router.PointerMove(AZ::Vector2(0.3f, 0.3f), targets);
        EXPECT_EQ(m_bottom.m_calls.size(), 1u);
    }

    TEST_F(InputRouterTests, UnhandledPressClearsFocusAndKeysGoToTopmost)
    {
        InputTarget* targets[] = { &m_bottom, &m_top };
        m_top.m_handles = false;
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, targets);   // bottom takes focus
        m_bottom.m_handles = false;
        EXPECT_FALSE(m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, targets));
        m_top.m_calls.clear();
        m_router.Key(Noesis::Key_B, true, targets);
        EXPECT_EQ(m_top.m_calls.size(), 1u);
    }

    TEST_F(InputRouterTests, VanishedTargetsAreNeverCalled)
    {
        InputTarget* both[] = { &m_bottom, &m_top };
        InputTarget* onlyBottom[] = { &m_bottom };
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, both);    // top captures
        m_top.m_calls.clear();
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, false, onlyBottom);
        m_router.Key(Noesis::Key_C, true, onlyBottom);
        EXPECT_TRUE(m_top.m_calls.empty());
        EXPECT_EQ(m_bottom.m_calls.size(), 2u);
    }

    TEST_F(InputRouterTests, TouchCapturesPerId)
    {
        InputTarget* targets[] = { &m_bottom, &m_top };
        m_top.m_handles = false;
        m_router.Touch(AZ::Vector2(), 1, TouchPhase::Down, targets);   // bottom captures id 1
        m_top.m_handles = true;
        m_router.Touch(AZ::Vector2(), 2, TouchPhase::Down, targets);   // top captures id 2
        m_top.m_calls.clear();
        m_bottom.m_calls.clear();
        m_router.Touch(AZ::Vector2(), 1, TouchPhase::Move, targets);
        m_router.Touch(AZ::Vector2(), 2, TouchPhase::Up, targets);
        EXPECT_EQ(m_bottom.m_calls, AZStd::vector<AZStd::string>({ "tmove 1" }));
        EXPECT_EQ(m_top.m_calls, AZStd::vector<AZStd::string>({ "tup 2" }));
    }

    TEST_F(InputRouterTests, TextDecodesUtf8ToCodepoints)
    {
        InputTarget* targets[] = { &m_top };
        EXPECT_TRUE(m_router.Text("a\xC3\xA9", targets));   // "aé"
        EXPECT_EQ(m_top.m_calls, AZStd::vector<AZStd::string>({ "char 97", "char 233" }));
    }

    TEST_F(InputRouterTests, NoTargetsMeansNotConsumed)
    {
        InputTargets empty;
        EXPECT_FALSE(m_router.PointerMove(AZ::Vector2(), empty));
        EXPECT_FALSE(m_router.Key(Noesis::Key_A, true, empty));
        EXPECT_FALSE(m_router.Text("x", empty));
    }

    TEST_F(InputRouterTests, WheelGoesTopDownUntilHandled)
    {
        m_top.m_handles = false;
        InputTarget* targets[] = { &m_bottom, &m_top };
        EXPECT_TRUE(m_router.MouseWheel(AZ::Vector2(0.5f, 0.5f), 3, targets));
        EXPECT_EQ(m_top.m_calls.size(), 1u);
        ASSERT_EQ(m_bottom.m_calls.size(), 1u);
        EXPECT_NE(m_bottom.m_calls[0].find("3"), AZStd::string::npos);
    }

    TEST_F(InputRouterTests, ReleaseWithoutCapturerClearsCapture)
    {
        InputTarget* both[] = { &m_bottom, &m_top };
        InputTarget* onlyBottom[] = { &m_bottom };

        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, both);   // top captures
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, false, onlyBottom);   // capturer (top) absent

        m_top.m_calls.clear();
        m_bottom.m_calls.clear();
        m_router.PointerMove(AZ::Vector2(0.1f, 0.1f), both);
        EXPECT_EQ(m_bottom.m_calls.size(), 1u);
        EXPECT_EQ(m_top.m_calls.size(), 1u);   // capture was cleared, so the move reaches both targets again

        m_top.m_calls.clear();
        m_bottom.m_calls.clear();
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, false, both);
        EXPECT_EQ(m_top.m_calls.size(), 1u);   // one MouseButtonUp offered top-down, not a phantom second from stale capture
    }

    TEST_F(InputRouterTests, ForgetClearsFocusAndCapture)
    {
        InputTarget* both[] = { &m_bottom, &m_top };
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, both);   // top takes focus and capture
        m_router.Forget(&m_top);

        m_top.m_calls.clear();
        m_bottom.m_calls.clear();
        m_router.PointerMove(AZ::Vector2(0.4f, 0.4f), both);
        EXPECT_EQ(m_top.m_calls.size(), 1u);
        EXPECT_EQ(m_bottom.m_calls.size(), 1u);   // capture forgotten: move broadcasts to both instead of routing only to top

        m_top.m_calls.clear();
        m_bottom.m_calls.clear();
        InputTarget* topNotLast[] = { &m_top, &m_bottom };   // top still present, but no longer the topmost element
        m_router.Key(Noesis::Key_D, true, topNotLast);
        EXPECT_TRUE(m_top.m_calls.empty());       // focus cleared: KeyTarget doesn't return the forgotten target directly
        EXPECT_EQ(m_bottom.m_calls.size(), 1u);   // falls back to the actual topmost element instead
    }

    TEST_F(InputRouterTests, PressReplacesExistingCapture)
    {
        InputTarget* targets[] = { &m_bottom, &m_top };
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, targets);   // top captures

        m_top.m_handles = false;
        m_router.MouseButton(AZ::Vector2(), Noesis::MouseButton_Left, true, targets);   // bottom now captures

        m_top.m_calls.clear();
        m_bottom.m_calls.clear();
        m_router.PointerMove(AZ::Vector2(0.3f, 0.3f), targets);
        EXPECT_TRUE(m_top.m_calls.empty());
        EXPECT_EQ(m_bottom.m_calls.size(), 1u);
    }
}
