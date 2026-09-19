#include <AzCore/UnitTest/TestTypes.h>
#include <AzFramework/Input/Devices/Gamepad/InputDeviceGamepad.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzTest/AzTest.h>
#include <Input/KeyTable.h>

namespace NoesisGUI::Test
{
    using namespace AzFramework;

    class KeyTableTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(KeyTableTests, EveryKeyboardChannelMaps)
    {
        for (const InputChannelId& channel : InputDeviceKeyboard::Key::All)
        {
            EXPECT_TRUE(KeyTable::ToNoesisKey(channel).has_value()) << channel.GetName();
        }
    }

    TEST_F(KeyTableTests, RepresentativeKeyboardKeys)
    {
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::AlphanumericA), Noesis::Key_A);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::Alphanumeric7), Noesis::Key_D7);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::EditEnter), Noesis::Key_Return);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::NavigationPageUp), Noesis::Key_PageUp);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::ModifierCtrlR), Noesis::Key_RightCtrl);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::Function12), Noesis::Key_F12);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::PunctuationSemicolon), Noesis::Key_OemSemicolon);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceKeyboard::Key::NumPadEnter), Noesis::Key_Return);
    }

    TEST_F(KeyTableTests, GamepadMatchesSdkDisplayTable)
    {
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::A), Noesis::Key_GamepadAccept);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::B), Noesis::Key_GamepadCancel);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::X), Noesis::Key_GamepadContext1);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::Y), Noesis::Key_GamepadContext2);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::L3), Noesis::Key_GamepadContext3);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::R3), Noesis::Key_GamepadContext4);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::L1), Noesis::Key_GamepadPageLeft);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::R1), Noesis::Key_GamepadPageRight);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::Select), Noesis::Key_GamepadView);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::Start), Noesis::Key_GamepadMenu);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Trigger::L2), Noesis::Key_GamepadPageUp);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Trigger::R2), Noesis::Key_GamepadPageDown);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::DU), Noesis::Key_GamepadUp);
        EXPECT_EQ(KeyTable::ToNoesisKey(InputDeviceGamepad::Button::DL), Noesis::Key_GamepadLeft);
    }

    TEST_F(KeyTableTests, MouseButtonsAndUnmappedChannels)
    {
        EXPECT_EQ(KeyTable::ToNoesisMouseButton(InputDeviceMouse::Button::Left), Noesis::MouseButton_Left);
        EXPECT_EQ(KeyTable::ToNoesisMouseButton(InputDeviceMouse::Button::Right), Noesis::MouseButton_Right);
        EXPECT_EQ(KeyTable::ToNoesisMouseButton(InputDeviceMouse::Button::Middle), Noesis::MouseButton_Middle);
        EXPECT_FALSE(KeyTable::ToNoesisMouseButton(InputDeviceKeyboard::Key::AlphanumericA).has_value());
        EXPECT_FALSE(KeyTable::ToNoesisKey(InputDeviceMouse::Button::Left).has_value());
    }
}
