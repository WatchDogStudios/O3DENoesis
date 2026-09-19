#include <AzCore/Math/Color.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/string/string.h>
#include <AzTest/AzTest.h>
#include <Binding/ValueConversion.h>

#include <NsCore/BaseComponent.h>
#include <NsCore/Boxing.h>
#include <NsCore/String.h>
#include <NsDrawing/Color.h>
#include <NsDrawing/Point.h>
#include <NsDrawing/Thickness.h>

namespace NoesisGUI::Test
{
    class ValueConversionTests : public UnitTest::LeakDetectionFixture
    {
    protected:
        template<class T>
        T RoundTrip(const T& in)
        {
            Noesis::Ptr<Noesis::BaseComponent> boxed = ValueConversion::ToNoesis(azrtti_typeid<T>(), &in);
            EXPECT_TRUE(boxed);
            T out{};
            EXPECT_TRUE(ValueConversion::FromNoesis(boxed, azrtti_typeid<T>(), &out));
            return out;
        }
    };

    TEST_F(ValueConversionTests, PrimitivesRoundTrip)
    {
        EXPECT_EQ(RoundTrip(true), true);
        EXPECT_EQ(RoundTrip<AZ::s8>(-7), -7);
        EXPECT_EQ(RoundTrip<AZ::s16>(-300), -300);
        EXPECT_EQ(RoundTrip<AZ::s32>(-70000), -70000);
        EXPECT_EQ(RoundTrip<AZ::s64>(-5000000000ll), -5000000000ll);
        EXPECT_EQ(RoundTrip<AZ::u8>(200), 200);
        EXPECT_EQ(RoundTrip<AZ::u16>(60000), 60000);
        EXPECT_EQ(RoundTrip<AZ::u32>(4000000000u), 4000000000u);
        EXPECT_EQ(RoundTrip<AZ::u64>(9000000000ull), 9000000000ull);
        EXPECT_FLOAT_EQ(RoundTrip(1.25f), 1.25f);
        EXPECT_DOUBLE_EQ(RoundTrip(2.5), 2.5);
    }

    TEST_F(ValueConversionTests, StringRoundTripsAndBoxesAsNoesisString)
    {
        const AZStd::string text = "Hello";
        Noesis::Ptr<Noesis::BaseComponent> boxed = ValueConversion::ToNoesis(azrtti_typeid<AZStd::string>(), &text);
        ASSERT_TRUE(Noesis::Boxing::CanUnbox<Noesis::String>(boxed));
        EXPECT_EQ(RoundTrip(text), text);
    }

    TEST_F(ValueConversionTests, MathTypesMapToNoesisTypes)
    {
        const AZ::Color color(0.1f, 0.2f, 0.3f, 0.4f);
        Noesis::Ptr<Noesis::BaseComponent> colorBoxed = ValueConversion::ToNoesis(azrtti_typeid<AZ::Color>(), &color);
        ASSERT_TRUE(Noesis::Boxing::CanUnbox<Noesis::Color>(colorBoxed));
        const Noesis::Color unboxedColor = Noesis::Boxing::Unbox<Noesis::Color>(colorBoxed);
        EXPECT_FLOAT_EQ(unboxedColor.r, color.GetR());
        EXPECT_FLOAT_EQ(unboxedColor.g, color.GetG());
        EXPECT_FLOAT_EQ(unboxedColor.b, color.GetB());
        EXPECT_FLOAT_EQ(unboxedColor.a, color.GetA());
        EXPECT_TRUE(RoundTrip(color).IsClose(color));

        const AZ::Vector2 point(3.0f, 4.0f);
        Noesis::Ptr<Noesis::BaseComponent> pointBoxed = ValueConversion::ToNoesis(azrtti_typeid<AZ::Vector2>(), &point);
        ASSERT_TRUE(Noesis::Boxing::CanUnbox<Noesis::Point>(pointBoxed));
        const Noesis::Point unboxedPoint = Noesis::Boxing::Unbox<Noesis::Point>(pointBoxed);
        EXPECT_FLOAT_EQ(unboxedPoint.x, point.GetX());
        EXPECT_FLOAT_EQ(unboxedPoint.y, point.GetY());
        EXPECT_TRUE(RoundTrip(point).IsClose(point));

        const AZ::Vector4 thickness(1.0f, 2.0f, 3.0f, 4.0f);
        Noesis::Ptr<Noesis::BaseComponent> thicknessBoxed = ValueConversion::ToNoesis(azrtti_typeid<AZ::Vector4>(), &thickness);
        ASSERT_TRUE(Noesis::Boxing::CanUnbox<Noesis::Thickness>(thicknessBoxed));
        const Noesis::Thickness unboxedThickness = Noesis::Boxing::Unbox<Noesis::Thickness>(thicknessBoxed);
        EXPECT_FLOAT_EQ(unboxedThickness.left, thickness.GetX());
        EXPECT_FLOAT_EQ(unboxedThickness.top, thickness.GetY());
        EXPECT_FLOAT_EQ(unboxedThickness.right, thickness.GetZ());
        EXPECT_FLOAT_EQ(unboxedThickness.bottom, thickness.GetW());
        EXPECT_TRUE(RoundTrip(thickness).IsClose(thickness));
    }

    TEST_F(ValueConversionTests, UnsupportedAndMismatchedFail)
    {
        const AZ::TypeId unknown("{11111111-2222-3333-4444-555555555555}");
        EXPECT_FALSE(ValueConversion::IsSupported(unknown));
        EXPECT_TRUE(ValueConversion::IsSupported(azrtti_typeid<AZStd::string>()));
        EXPECT_TRUE(ValueConversion::IsSupported(azrtti_typeid<AZ::u64>()));
        int dummy = 0;
        EXPECT_FALSE(ValueConversion::ToNoesis(unknown, &dummy));

        const AZStd::string text = "not a number";
        Noesis::Ptr<Noesis::BaseComponent> boxed = ValueConversion::ToNoesis(azrtti_typeid<AZStd::string>(), &text);
        float out = 0.0f;
        EXPECT_FALSE(ValueConversion::FromNoesis(boxed, azrtti_typeid<float>(), &out));
        EXPECT_FALSE(ValueConversion::FromNoesis(nullptr, azrtti_typeid<float>(), &out));
    }

    TEST_F(ValueConversionTests, MakeImageNeedsUri)
    {
        EXPECT_FALSE(ValueConversion::MakeImage(""));
        EXPECT_TRUE(ValueConversion::MakeImage("Images/Logo.png"));
    }
}
