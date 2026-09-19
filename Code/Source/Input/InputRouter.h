#pragma once

#include <AzCore/Math/Vector2.h>
#include <AzCore/std/containers/fixed_vector.h>
#include <AzCore/std/containers/span.h>
#include <AzCore/std/string/string_view.h>
#include <AzCore/std/utility/pair.h>

#include <NsGui/InputEnums.h>

namespace NoesisGUI
{
    class InputTarget
    {
    public:
        virtual ~InputTarget() = default;
        virtual bool MouseMove(int x, int y) = 0;
        virtual bool MouseButtonDown(int x, int y, Noesis::MouseButton button) = 0;
        virtual bool MouseButtonUp(int x, int y, Noesis::MouseButton button) = 0;
        virtual bool MouseWheel(int x, int y, int wheelRotation) = 0;
        virtual bool TouchDown(int x, int y, AZ::u64 id) = 0;
        virtual bool TouchMove(int x, int y, AZ::u64 id) = 0;
        virtual bool TouchUp(int x, int y, AZ::u64 id) = 0;
        virtual bool KeyDown(Noesis::Key key) = 0;
        virtual bool KeyUp(Noesis::Key key) = 0;
        virtual bool Char(AZ::u32 codepoint) = 0;
    };

    enum class TouchPhase : AZ::u8
    {
        Down,
        Move,
        Up
    };

    using InputTargets = AZStd::span<InputTarget* const>;

    class InputRouter
    {
    public:
        void SetViewportSize(AZ::u32 width, AZ::u32 height);
        bool PointerMove(const AZ::Vector2& normalizedPosition, InputTargets targets);
        bool MouseButton(const AZ::Vector2& normalizedPosition, Noesis::MouseButton button, bool pressed, InputTargets targets);
        bool MouseWheel(const AZ::Vector2& normalizedPosition, int wheelRotation, InputTargets targets);
        bool Touch(const AZ::Vector2& normalizedPosition, AZ::u64 id, TouchPhase phase, InputTargets targets);
        bool Key(Noesis::Key key, bool pressed, InputTargets targets);
        bool Text(AZStd::string_view utf8, InputTargets targets);
        void Forget(const InputTarget* target);

    private:
        AZStd::pair<int, int> ToPixels(const AZ::Vector2& normalizedPosition) const;
        static bool Contains(InputTargets targets, const InputTarget* target);
        InputTarget* KeyTarget(InputTargets targets) const;

        AZ::u32 m_width = 0;
        AZ::u32 m_height = 0;
        InputTarget* m_focus = nullptr;
        InputTarget* m_capture = nullptr;
        // ponytail: 10 simultaneous touches, matching InputDeviceTouch::Touch::Index0..Index9.
        AZStd::fixed_vector<AZStd::pair<AZ::u64, InputTarget*>, 10> m_touchCaptures;
    };
}
