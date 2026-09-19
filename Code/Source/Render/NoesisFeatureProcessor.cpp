#include <Render/NoesisFeatureProcessor.h>

#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzFramework/Input/Channels/InputChannel.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Input/Devices/Touch/InputDeviceTouch.h>
#include <Input/KeyTable.h>

#include <NsGui/IRenderer.h>

namespace NoesisGUI
{
    namespace
    {
        // ponytail: one lock for all scenes' Noesis updates; Noesis global state (providers, caches) is not
        // thread-safe and feature processors of different scenes may simulate concurrently.
        AZStd::mutex s_noesisUpdateMutex;

        class ViewInputTarget final : public InputTarget
        {
        public:
            explicit ViewInputTarget(Noesis::IView* view)
                : m_view(view)
            {
            }

            bool MouseMove(int x, int y) override { return m_view->MouseMove(x, y); }
            bool MouseButtonDown(int x, int y, Noesis::MouseButton button) override { return m_view->MouseButtonDown(x, y, button); }
            bool MouseButtonUp(int x, int y, Noesis::MouseButton button) override { return m_view->MouseButtonUp(x, y, button); }
            bool MouseWheel(int x, int y, int wheelRotation) override { return m_view->MouseWheel(x, y, wheelRotation); }
            bool TouchDown(int x, int y, AZ::u64 id) override { return m_view->TouchDown(x, y, id); }
            bool TouchMove(int x, int y, AZ::u64 id) override { return m_view->TouchMove(x, y, id); }
            bool TouchUp(int x, int y, AZ::u64 id) override { return m_view->TouchUp(x, y, id); }
            bool KeyDown(Noesis::Key key) override { return m_view->KeyDown(key); }
            bool KeyUp(Noesis::Key key) override { return m_view->KeyUp(key); }
            bool Char(AZ::u32 codepoint) override { return m_view->Char(codepoint); }

        private:
            Noesis::IView* m_view;
        };
    }

    AZStd::mutex& NoesisFeatureProcessor::GetUpdateMutex()
    {
        return s_noesisUpdateMutex;
    }

    void NoesisFeatureProcessor::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisFeatureProcessor, AZ::RPI::FeatureProcessor>();
        }
    }

    void NoesisFeatureProcessor::Activate()
    {
        m_device = *new AtomRenderDevice();
        AZ_Error("NoesisGUI", m_device->Init(), "Some Noesis shaders failed to load; affected UI elements will not draw.");
        m_inputConnection = EngineHooks::ConnectInput(
            [this](const AzFramework::InputChannel& channel) { return OnInputChannel(channel); },
            [this](const AZStd::string& text) { return OnInputText(text); });
        EnableSceneNotification();
    }

    void NoesisFeatureProcessor::Deactivate()
    {
        DisableSceneNotification();
        m_inputConnection.reset();
        {
            AZStd::scoped_lock lock(s_noesisUpdateMutex);
            for (ViewEntry& entry : m_views)
            {
                entry.m_view->GetRenderer()->Shutdown();
            }
            m_views.clear();
            m_inputTargets.clear();
            m_device->Shutdown();
        }
        m_device.Reset();
    }

    NoesisFeatureProcessor::ViewEntry* NoesisFeatureProcessor::FindView(const Noesis::IView* view)
    {
        auto it = AZStd::find_if(m_views.begin(), m_views.end(), [view](const ViewEntry& entry) { return entry.m_view.GetPtr() == view; });
        return it != m_views.end() ? &*it : nullptr;
    }

    void NoesisFeatureProcessor::RebuildInputTargets()
    {
        m_inputTargets.clear();
        for (ViewEntry& entry : m_views)
        {
            if (entry.m_visible && entry.m_inputEnabled)
            {
                m_inputTargets.push_back(entry.m_input.get());
            }
        }
    }

    void NoesisFeatureProcessor::AddView(Noesis::IView* view, AZ::s32 zOrder, bool visible, bool inputEnabled)
    {
        AZStd::scoped_lock lock(s_noesisUpdateMutex);
        view->GetRenderer()->Init(m_device.GetPtr());
        auto it = AZStd::upper_bound(m_views.begin(), m_views.end(), zOrder,
            [](AZ::s32 z, const ViewEntry& entry) { return z < entry.m_zOrder; });
        ViewEntry entry;
        entry.m_view = Noesis::Ptr<Noesis::IView>(view);
        entry.m_zOrder = zOrder;
        entry.m_visible = visible;
        entry.m_inputEnabled = inputEnabled;
        entry.m_input = AZStd::make_unique<ViewInputTarget>(view);
        m_views.insert(it, AZStd::move(entry));
        RebuildInputTargets();
    }

    void NoesisFeatureProcessor::RemoveView(Noesis::IView* view)
    {
        AZStd::scoped_lock lock(s_noesisUpdateMutex);
        auto it = AZStd::find_if(m_views.begin(), m_views.end(), [view](const ViewEntry& entry) { return entry.m_view.GetPtr() == view; });
        if (it != m_views.end())
        {
            m_router.Forget(it->m_input.get());
            view->GetRenderer()->Shutdown();
            m_views.erase(it);
            RebuildInputTargets();
        }
    }

    void NoesisFeatureProcessor::SetViewVisible(Noesis::IView* view, bool visible)
    {
        AZStd::scoped_lock lock(s_noesisUpdateMutex);
        if (ViewEntry* entry = FindView(view))
        {
            entry->m_visible = visible;
            RebuildInputTargets();
        }
    }

    void NoesisFeatureProcessor::SetViewInputEnabled(Noesis::IView* view, bool inputEnabled)
    {
        AZStd::scoped_lock lock(s_noesisUpdateMutex);
        if (ViewEntry* entry = FindView(view))
        {
            entry->m_inputEnabled = inputEnabled;
            RebuildInputTargets();
        }
    }

    // ponytail: normalized cursor × pass output size; the Editor game-mode viewport's offset inside the main window is ignored.
    void NoesisFeatureProcessor::SetOutputSize(AZ::u32 width, AZ::u32 height)
    {
        m_router.SetViewportSize(width, height);
    }

    bool NoesisFeatureProcessor::OnInputChannel(const AzFramework::InputChannel& channel)
    {
        AZStd::scoped_lock lock(s_noesisUpdateMutex);
        if (m_inputTargets.empty())
        {
            return false;
        }

        const AzFramework::InputChannelId& id = channel.GetInputChannelId();
        const auto* position = channel.GetCustomData<AzFramework::InputChannel::PositionData2D>();
        const bool edge = channel.IsStateBegan() || channel.IsStateEnded();

        if (id == AzFramework::InputDeviceMouse::SystemCursorPosition)
        {
            if (!position || !channel.IsActive())
            {
                return false;
            }
            m_lastPointer = position->m_normalizedPosition;
            return m_router.PointerMove(m_lastPointer, m_inputTargets);
        }
        if (AZStd::optional<Noesis::MouseButton> button = KeyTable::ToNoesisMouseButton(id))
        {
            return edge && m_router.MouseButton(position ? position->m_normalizedPosition : m_lastPointer, *button, channel.IsStateBegan(), m_inputTargets);
        }
        if (id == AzFramework::InputDeviceMouse::Movement::Z)
        {
            // InputDeviceMouse doesn't document Movement::Z's units (WHEEL_DELTA notches vs. fractional deltas);
            // accumulate and truncate so sub-1.0 deltas aren't silently dropped.
            m_wheelRemainder += channel.GetValue();
            const int rotation = static_cast<int>(m_wheelRemainder);
            m_wheelRemainder -= rotation;
            return rotation != 0 && m_router.MouseWheel(m_lastPointer, rotation, m_inputTargets);
        }
        for (AZ::u64 index = 0; index < AzFramework::InputDeviceTouch::Touch::All.size(); ++index)
        {
            if (id == AzFramework::InputDeviceTouch::Touch::All[index])
            {
                if (!position)
                {
                    return false;
                }
                const TouchPhase phase = channel.IsStateBegan() ? TouchPhase::Down : channel.IsStateEnded() ? TouchPhase::Up : TouchPhase::Move;
                return m_router.Touch(position->m_normalizedPosition, index, phase, m_inputTargets);
            }
        }
        // ponytail: gamepad keys fire on press/release only; add a repeat timer (SDK: 250 ms delay, 31 Hz) if held navigation is needed.
        if (AZStd::optional<Noesis::Key> key = KeyTable::ToNoesisKey(id))
        {
            return edge && m_router.Key(*key, channel.IsStateBegan(), m_inputTargets);
        }
        return false;
    }

    bool NoesisFeatureProcessor::OnInputText(const AZStd::string& text)
    {
        AZStd::scoped_lock lock(s_noesisUpdateMutex);
        return m_router.Text(text, m_inputTargets);
    }

    void NoesisFeatureProcessor::AddRenderPasses(AZ::RPI::RenderPipeline* renderPipeline)
    {
        static const AZ::Name passName("NoesisPass");
        static const AZ::Name anchorName("ImGuiPass");
        if (renderPipeline->GetViewType() != AZ::RPI::ViewType::Default || renderPipeline->FindFirstPass(passName))
        {
            return;
        }

        AZ::RPI::PassRequest request;
        request.m_passName = passName;
        request.m_templateName = AZ::Name("NoesisPassTemplate");
        request.AddInputConnection(AZ::RPI::PassConnection{ AZ::Name("InputOutput"), AZ::RPI::PassAttachmentRef{ anchorName, AZ::Name("InputOutput") } });

        AZ::RPI::Ptr<AZ::RPI::Pass> pass = AZ::RPI::PassSystemInterface::Get()->CreatePassFromRequest(&request);
        const bool added = pass && renderPipeline->AddPassAfter(pass, anchorName);
        AZ_Warning("NoesisGUI", added, "Could not add NoesisPass to pipeline '%s' (no ImGuiPass anchor?). Noesis views will not render there.",
            renderPipeline->GetId().GetCStr());
    }
}
