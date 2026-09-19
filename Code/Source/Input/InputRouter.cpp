#include <Input/InputRouter.h>

#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/string/utf8/unchecked.h>

namespace NoesisGUI
{
    void InputRouter::SetViewportSize(AZ::u32 width, AZ::u32 height)
    {
        m_width = width;
        m_height = height;
    }

    AZStd::pair<int, int> InputRouter::ToPixels(const AZ::Vector2& normalizedPosition) const
    {
        return { static_cast<int>(normalizedPosition.GetX() * m_width), static_cast<int>(normalizedPosition.GetY() * m_height) };
    }

    bool InputRouter::Contains(InputTargets targets, const InputTarget* target)
    {
        return target && AZStd::find(targets.begin(), targets.end(), target) != targets.end();
    }

    InputTarget* InputRouter::KeyTarget(InputTargets targets) const
    {
        if (Contains(targets, m_focus))
        {
            return m_focus;
        }
        return targets.empty() ? nullptr : targets.back();
    }

    bool InputRouter::PointerMove(const AZ::Vector2& normalizedPosition, InputTargets targets)
    {
        const auto [x, y] = ToPixels(normalizedPosition);
        if (Contains(targets, m_capture))
        {
            return m_capture->MouseMove(x, y);
        }
        bool handled = false;
        for (auto it = targets.rbegin(); it != targets.rend(); ++it)
        {
            handled |= (*it)->MouseMove(x, y);
        }
        return handled;
    }

    bool InputRouter::MouseButton(const AZ::Vector2& normalizedPosition, Noesis::MouseButton button, bool pressed, InputTargets targets)
    {
        const auto [x, y] = ToPixels(normalizedPosition);
        if (!pressed)
        {
            InputTarget* capture = m_capture;
            m_capture = nullptr;
            if (Contains(targets, capture))
            {
                return capture->MouseButtonUp(x, y, button);
            }
        }
        for (auto it = targets.rbegin(); it != targets.rend(); ++it)
        {
            if (pressed ? (*it)->MouseButtonDown(x, y, button) : (*it)->MouseButtonUp(x, y, button))
            {
                if (pressed)
                {
                    m_focus = *it;
                    m_capture = *it;
                }
                return true;
            }
        }
        if (pressed)
        {
            m_focus = nullptr;
        }
        return false;
    }

    bool InputRouter::MouseWheel(const AZ::Vector2& normalizedPosition, int wheelRotation, InputTargets targets)
    {
        const auto [x, y] = ToPixels(normalizedPosition);
        for (auto it = targets.rbegin(); it != targets.rend(); ++it)
        {
            if ((*it)->MouseWheel(x, y, wheelRotation))
            {
                return true;
            }
        }
        return false;
    }

    bool InputRouter::Touch(const AZ::Vector2& normalizedPosition, AZ::u64 id, TouchPhase phase, InputTargets targets)
    {
        const auto [x, y] = ToPixels(normalizedPosition);

        if (phase != TouchPhase::Down)
        {
            auto captured = AZStd::find_if(m_touchCaptures.begin(), m_touchCaptures.end(), [id](const auto& entry) { return entry.first == id; });
            if (captured != m_touchCaptures.end())
            {
                InputTarget* target = captured->second;
                if (phase == TouchPhase::Up)
                {
                    m_touchCaptures.erase(captured);
                }
                if (!Contains(targets, target))
                {
                    return false;
                }
                return phase == TouchPhase::Move ? target->TouchMove(x, y, id) : target->TouchUp(x, y, id);
            }
        }

        for (auto it = targets.rbegin(); it != targets.rend(); ++it)
        {
            const bool handled = phase == TouchPhase::Down ? (*it)->TouchDown(x, y, id)
                : phase == TouchPhase::Move               ? (*it)->TouchMove(x, y, id)
                                                          : (*it)->TouchUp(x, y, id);
            if (handled)
            {
                if (phase == TouchPhase::Down)
                {
                    // Replace any stale entry for this id (e.g. a Down without a matching Up) so captures never duplicate.
                    AZStd::erase_if(m_touchCaptures, [id](const auto& entry) { return entry.first == id; });
                    if (m_touchCaptures.size() < m_touchCaptures.capacity())
                    {
                        m_touchCaptures.emplace_back(id, *it);
                    }
                    m_focus = *it;
                }
                return true;
            }
        }
        return false;
    }

    bool InputRouter::Key(Noesis::Key key, bool pressed, InputTargets targets)
    {
        InputTarget* target = KeyTarget(targets);
        if (!target)
        {
            return false;
        }
        return pressed ? target->KeyDown(key) : target->KeyUp(key);
    }

    bool InputRouter::Text(AZStd::string_view utf8, InputTargets targets)
    {
        InputTarget* target = KeyTarget(targets);
        if (!target)
        {
            return false;
        }
        // ponytail: 64 code points per text event; one event per keystroke or IME commit is far below that.
        AZStd::array<AZ::u32, 64> codepoints;
        const auto end = Utf8::Unchecked::utf8to32(utf8.begin(), utf8.end(), codepoints.begin(), codepoints.size());
        bool handled = false;
        for (auto it = codepoints.begin(); it != end; ++it)
        {
            handled |= target->Char(*it);
        }
        return handled;
    }

    void InputRouter::Forget(const InputTarget* target)
    {
        if (m_focus == target)
        {
            m_focus = nullptr;
        }
        if (m_capture == target)
        {
            m_capture = nullptr;
        }
        AZStd::erase_if(m_touchCaptures, [target](const auto& entry) { return entry.second == target; });
    }
}
