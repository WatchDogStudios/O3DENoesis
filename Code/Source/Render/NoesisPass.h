#pragma once

#include <Atom/RHI.Reflect/Viewport.h>
#include <Atom/RHI/ScopeProducerFunction.h>
#include <Atom/RPI.Public/Pass/RenderPass.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <Render/AtomRenderDevice.h>

namespace NoesisGUI
{
    class NoesisPass final : public AZ::RPI::RenderPass
    {
        using Base = AZ::RPI::RenderPass;
        AZ_RPI_PASS(NoesisPass);

    public:
        AZ_RTTI(NoesisGUI::NoesisPass, "{7C2E5A93-1D4B-4F8E-B6A0-9E3D2C1F5B64}", Base);
        AZ_CLASS_ALLOCATOR(NoesisPass, AZ::SystemAllocator);

        static AZ::RPI::Ptr<NoesisPass> Create(const AZ::RPI::PassDescriptor& descriptor);

    private:
        explicit NoesisPass(const AZ::RPI::PassDescriptor& descriptor);

        void FrameBeginInternal(FramePrepareParams params) override;
        void SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph) override;
        void BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context) override;

        struct OffscreenScope
        {
            NoesisPass* m_pass = nullptr;
            AZ::u32 m_tapeScope = 0;
        };

        struct OffscreenFunctions
        {
            static void Prepare(AZ::RHI::FrameGraphInterface frameGraph, OffscreenScope& scope);
            static void Execute(const AZ::RHI::FrameGraphExecuteContext& context, const OffscreenScope& scope);
        };

        using Producer = AZ::RHI::ScopeProducerFunction<
            OffscreenScope, decltype(&OffscreenFunctions::Prepare), AZ::RHI::EmptyCompileFunction<OffscreenScope>,
            decltype(&OffscreenFunctions::Execute)>;

        static void Import(AZ::RHI::FrameGraphInterface& frameGraph, AZ::RPI::AttachmentImage* image);
        static void DeclareSampled(AZ::RHI::FrameGraphInterface& frameGraph, const AZStd::vector<const AtomTexture*>& textures);
        static void Submit(const AZ::RHI::FrameGraphExecuteContext& context, const TapeDraw& draw, AZ::u32 submitIndex);
        Producer& AcquireProducer(size_t index);

        NoesisTape m_tape;
        AZStd::vector<AZ::u32> m_onscreenDraws;
        AZStd::vector<AZ::u32> m_onscreenScopes;
        // Producers keep their address and ScopeId across frames; only their payload changes (frame-graph identity).
        AZStd::vector<AZStd::unique_ptr<Producer>> m_producers;
        size_t m_producerCount = 0;
        AZ::RHI::Viewport m_viewport;
    };
}
