#include <Input/KeyTable.h>

#include <AzFramework/Input/Devices/Gamepad/InputDeviceGamepad.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>

namespace NoesisGUI::KeyTable
{
    namespace
    {
        using AzFramework::InputChannelId;
        using Keyboard = AzFramework::InputDeviceKeyboard::Key;
        using Gamepad = AzFramework::InputDeviceGamepad;
        using Mouse = AzFramework::InputDeviceMouse::Button;

        struct KeyEntry
        {
            InputChannelId m_channel;
            Noesis::Key m_key;
        };

        // A constexpr array instead of a map: a static AZStd container would outlive the allocators at shutdown.
        constexpr KeyEntry Keys[] = {
            { Keyboard::Alphanumeric0, Noesis::Key_D0 }, { Keyboard::Alphanumeric1, Noesis::Key_D1 },
            { Keyboard::Alphanumeric2, Noesis::Key_D2 }, { Keyboard::Alphanumeric3, Noesis::Key_D3 },
            { Keyboard::Alphanumeric4, Noesis::Key_D4 }, { Keyboard::Alphanumeric5, Noesis::Key_D5 },
            { Keyboard::Alphanumeric6, Noesis::Key_D6 }, { Keyboard::Alphanumeric7, Noesis::Key_D7 },
            { Keyboard::Alphanumeric8, Noesis::Key_D8 }, { Keyboard::Alphanumeric9, Noesis::Key_D9 },
            { Keyboard::AlphanumericA, Noesis::Key_A }, { Keyboard::AlphanumericB, Noesis::Key_B },
            { Keyboard::AlphanumericC, Noesis::Key_C }, { Keyboard::AlphanumericD, Noesis::Key_D },
            { Keyboard::AlphanumericE, Noesis::Key_E }, { Keyboard::AlphanumericF, Noesis::Key_F },
            { Keyboard::AlphanumericG, Noesis::Key_G }, { Keyboard::AlphanumericH, Noesis::Key_H },
            { Keyboard::AlphanumericI, Noesis::Key_I }, { Keyboard::AlphanumericJ, Noesis::Key_J },
            { Keyboard::AlphanumericK, Noesis::Key_K }, { Keyboard::AlphanumericL, Noesis::Key_L },
            { Keyboard::AlphanumericM, Noesis::Key_M }, { Keyboard::AlphanumericN, Noesis::Key_N },
            { Keyboard::AlphanumericO, Noesis::Key_O }, { Keyboard::AlphanumericP, Noesis::Key_P },
            { Keyboard::AlphanumericQ, Noesis::Key_Q }, { Keyboard::AlphanumericR, Noesis::Key_R },
            { Keyboard::AlphanumericS, Noesis::Key_S }, { Keyboard::AlphanumericT, Noesis::Key_T },
            { Keyboard::AlphanumericU, Noesis::Key_U }, { Keyboard::AlphanumericV, Noesis::Key_V },
            { Keyboard::AlphanumericW, Noesis::Key_W }, { Keyboard::AlphanumericX, Noesis::Key_X },
            { Keyboard::AlphanumericY, Noesis::Key_Y }, { Keyboard::AlphanumericZ, Noesis::Key_Z },
            { Keyboard::EditBackspace, Noesis::Key_Back }, { Keyboard::EditCapsLock, Noesis::Key_CapsLock },
            { Keyboard::EditEnter, Noesis::Key_Return }, { Keyboard::EditSpace, Noesis::Key_Space },
            { Keyboard::EditTab, Noesis::Key_Tab }, { Keyboard::Escape, Noesis::Key_Escape },
            { Keyboard::Function01, Noesis::Key_F1 }, { Keyboard::Function02, Noesis::Key_F2 },
            { Keyboard::Function03, Noesis::Key_F3 }, { Keyboard::Function04, Noesis::Key_F4 },
            { Keyboard::Function05, Noesis::Key_F5 }, { Keyboard::Function06, Noesis::Key_F6 },
            { Keyboard::Function07, Noesis::Key_F7 }, { Keyboard::Function08, Noesis::Key_F8 },
            { Keyboard::Function09, Noesis::Key_F9 }, { Keyboard::Function10, Noesis::Key_F10 },
            { Keyboard::Function11, Noesis::Key_F11 }, { Keyboard::Function12, Noesis::Key_F12 },
            { Keyboard::Function13, Noesis::Key_F13 }, { Keyboard::Function14, Noesis::Key_F14 },
            { Keyboard::Function15, Noesis::Key_F15 }, { Keyboard::Function16, Noesis::Key_F16 },
            { Keyboard::Function17, Noesis::Key_F17 }, { Keyboard::Function18, Noesis::Key_F18 },
            { Keyboard::Function19, Noesis::Key_F19 }, { Keyboard::Function20, Noesis::Key_F20 },
            { Keyboard::ModifierAltL, Noesis::Key_LeftAlt }, { Keyboard::ModifierAltR, Noesis::Key_RightAlt },
            { Keyboard::ModifierCtrlL, Noesis::Key_LeftCtrl }, { Keyboard::ModifierCtrlR, Noesis::Key_RightCtrl },
            { Keyboard::ModifierShiftL, Noesis::Key_LeftShift }, { Keyboard::ModifierShiftR, Noesis::Key_RightShift },
            { Keyboard::ModifierSuperL, Noesis::Key_LWin }, { Keyboard::ModifierSuperR, Noesis::Key_RWin },
            { Keyboard::NavigationArrowDown, Noesis::Key_Down }, { Keyboard::NavigationArrowLeft, Noesis::Key_Left },
            { Keyboard::NavigationArrowRight, Noesis::Key_Right }, { Keyboard::NavigationArrowUp, Noesis::Key_Up },
            { Keyboard::NavigationDelete, Noesis::Key_Delete }, { Keyboard::NavigationEnd, Noesis::Key_End },
            { Keyboard::NavigationHome, Noesis::Key_Home }, { Keyboard::NavigationInsert, Noesis::Key_Insert },
            { Keyboard::NavigationPageDown, Noesis::Key_PageDown }, { Keyboard::NavigationPageUp, Noesis::Key_PageUp },
            { Keyboard::NumLock, Noesis::Key_NumLock },
            { Keyboard::NumPad0, Noesis::Key_NumPad0 }, { Keyboard::NumPad1, Noesis::Key_NumPad1 },
            { Keyboard::NumPad2, Noesis::Key_NumPad2 }, { Keyboard::NumPad3, Noesis::Key_NumPad3 },
            { Keyboard::NumPad4, Noesis::Key_NumPad4 }, { Keyboard::NumPad5, Noesis::Key_NumPad5 },
            { Keyboard::NumPad6, Noesis::Key_NumPad6 }, { Keyboard::NumPad7, Noesis::Key_NumPad7 },
            { Keyboard::NumPad8, Noesis::Key_NumPad8 }, { Keyboard::NumPad9, Noesis::Key_NumPad9 },
            { Keyboard::NumPadAdd, Noesis::Key_Add }, { Keyboard::NumPadDecimal, Noesis::Key_Decimal },
            { Keyboard::NumPadDivide, Noesis::Key_Divide }, { Keyboard::NumPadEnter, Noesis::Key_Return },
            { Keyboard::NumPadMultiply, Noesis::Key_Multiply }, { Keyboard::NumPadSubtract, Noesis::Key_Subtract },
            { Keyboard::PunctuationApostrophe, Noesis::Key_OemQuotes }, { Keyboard::PunctuationBackslash, Noesis::Key_OemPipe },
            { Keyboard::PunctuationBracketL, Noesis::Key_OemOpenBrackets }, { Keyboard::PunctuationBracketR, Noesis::Key_OemCloseBrackets },
            { Keyboard::PunctuationComma, Noesis::Key_OemComma }, { Keyboard::PunctuationEquals, Noesis::Key_OemPlus },
            { Keyboard::PunctuationHyphen, Noesis::Key_OemMinus }, { Keyboard::PunctuationPeriod, Noesis::Key_OemPeriod },
            { Keyboard::PunctuationSemicolon, Noesis::Key_OemSemicolon }, { Keyboard::PunctuationSlash, Noesis::Key_OemQuestion },
            { Keyboard::PunctuationTilde, Noesis::Key_OemTilde }, { Keyboard::SupplementaryISO, Noesis::Key_OemBackslash },
            { Keyboard::WindowsSystemPause, Noesis::Key_Pause }, { Keyboard::WindowsSystemPrint, Noesis::Key_PrintScreen },
            { Keyboard::WindowsSystemScrollLock, Noesis::Key_Scroll },

            { Gamepad::Button::DL, Noesis::Key_GamepadLeft }, { Gamepad::Button::DR, Noesis::Key_GamepadRight },
            { Gamepad::Button::DU, Noesis::Key_GamepadUp }, { Gamepad::Button::DD, Noesis::Key_GamepadDown },
            { Gamepad::Button::A, Noesis::Key_GamepadAccept }, { Gamepad::Button::B, Noesis::Key_GamepadCancel },
            { Gamepad::Button::X, Noesis::Key_GamepadContext1 }, { Gamepad::Button::Y, Noesis::Key_GamepadContext2 },
            { Gamepad::Button::L3, Noesis::Key_GamepadContext3 }, { Gamepad::Button::R3, Noesis::Key_GamepadContext4 },
            { Gamepad::Button::L1, Noesis::Key_GamepadPageLeft }, { Gamepad::Button::R1, Noesis::Key_GamepadPageRight },
            { Gamepad::Button::Select, Noesis::Key_GamepadView }, { Gamepad::Button::Start, Noesis::Key_GamepadMenu },
            { Gamepad::Trigger::L2, Noesis::Key_GamepadPageUp }, { Gamepad::Trigger::R2, Noesis::Key_GamepadPageDown },
        };
    }

    AZStd::optional<Noesis::Key> ToNoesisKey(const InputChannelId& channelId)
    {
        for (const KeyEntry& entry : Keys)
        {
            if (entry.m_channel == channelId)
            {
                return entry.m_key;
            }
        }
        return AZStd::nullopt;
    }

    AZStd::optional<Noesis::MouseButton> ToNoesisMouseButton(const InputChannelId& channelId)
    {
        if (channelId == Mouse::Left)
        {
            return Noesis::MouseButton_Left;
        }
        if (channelId == Mouse::Right)
        {
            return Noesis::MouseButton_Right;
        }
        if (channelId == Mouse::Middle)
        {
            return Noesis::MouseButton_Middle;
        }
        if (channelId == Mouse::Other1)
        {
            return Noesis::MouseButton_XButton1;
        }
        if (channelId == Mouse::Other2)
        {
            return Noesis::MouseButton_XButton2;
        }
        return AZStd::nullopt;
    }
}
