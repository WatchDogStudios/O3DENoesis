#pragma once

#include <Atom/RPI.Public/FeatureProcessor.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <Input/InputRouter.h>
#include <Render/AtomRenderDevice.h>
#include <Runtime/EngineHooks.h>

#include <NsCore/Ptr.h>
#include <NsGui/IView.h>

namespace NoesisGUI
{
    class NoesisFeatureProcessor final : public AZ::RPI::FeatureProcessor
    {
    public:
        AZ_CLASS_ALLOCATOR(NoesisFeatureProcessor, AZ::SystemAllocator)
        AZ_RTTI(NoesisGUI::NoesisFeatureProcessor, "{2D7F9C41-6B1A-4E7D-9F0C-3A5B8E6D1C52}", AZ::RPI::FeatureProcessor);

        struct ViewEntry
        {
            Noesis::Ptr<Noesis::IView> m_view;
            AZ::s32 m_zOrder = 0;
            bool m_visible = true;
            bool m_inputEnabled = true;
            AZStd::unique_ptr<InputTarget> m_input;
        };

        static void Reflect(AZ::ReflectContext* context);

        //! The single mutex guarding all Noesis global state (providers, caches) across every scene's feature
        //! processor and pass. Exposed so NoesisPass::FrameBeginInternal can hold it while iterating GetViews()
        //! and driving the renderer, matching this class's own locking.
        static AZStd::mutex& GetUpdateMutex();

        void Activate() override;
        void Deactivate() override;
        void AddRenderPasses(AZ::RPI::RenderPipeline* renderPipeline) override;

        void AddView(Noesis::IView* view, AZ::s32 zOrder, bool visible, bool inputEnabled);
        void RemoveView(Noesis::IView* view);
        void SetViewVisible(Noesis::IView* view, bool visible);
        void SetViewInputEnabled(Noesis::IView* view, bool inputEnabled);
        //! Called by NoesisPass under GetUpdateMutex() with the size views are laid out at.
        void SetOutputSize(AZ::u32 width, AZ::u32 height);
        const AZStd::vector<ViewEntry>& GetViews() const { return m_views; }
        AtomRenderDevice& GetRenderDevice() { return *m_device; }

    private:
        ViewEntry* FindView(const Noesis::IView* view);
        void RebuildInputTargets();
        bool OnInputChannel(const AzFramework::InputChannel& channel);
        bool OnInputText(const AZStd::string& text);

        Noesis::Ptr<AtomRenderDevice> m_device;
        AZStd::vector<ViewEntry> m_views;
        AZStd::vector<InputTarget*> m_inputTargets;
        InputRouter m_router;
        AZ::Vector2 m_lastPointer = AZ::Vector2(0.5f, 0.5f);
        float m_wheelRemainder = 0.0f;
        AZStd::unique_ptr<EngineHooks::Connection> m_inputConnection;
    };
}
