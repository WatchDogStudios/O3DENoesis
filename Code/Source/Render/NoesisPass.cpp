#include <Render/NoesisPass.h>

#include <Atom/RHI.Reflect/AttachmentLoadStoreAction.h>
#include <Atom/RHI.Reflect/ImageScopeAttachmentDescriptor.h>
#include <Atom/RHI/CommandList.h>
#include <Atom/RHI/DeviceDrawItem.h>
#include <Atom/RHI/FrameGraphAttachmentInterface.h>
#include <Atom/RHI/FrameGraphBuilder.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Time/ITime.h>
#include <Providers/HotReload.h>
#include <Render/NoesisFeatureProcessor.h>
#include <Render/RenderStateMapping.h>

#include <NsGui/IRenderer.h>

namespace NoesisGUI
{
    using namespace AZ;

    RPI::Ptr<NoesisPass> NoesisPass::Create(const RPI::PassDescriptor& descriptor)
    {
        return aznew NoesisPass(descriptor);
    }

    NoesisPass::NoesisPass(const RPI::PassDescriptor& descriptor)
        : Base(descriptor)
    {
    }

    NoesisPass::Producer& NoesisPass::AcquireProducer(size_t index)
    {
        while (m_producers.size() <= index)
        {
            const RHI::ScopeId scopeId(AZStd::string::format("%s.Offscreen%zu", GetPathName().GetCStr(), m_producers.size()));
            m_producers.emplace_back(AZStd::make_unique<Producer>(
                scopeId, OffscreenScope{}, &OffscreenFunctions::Prepare, RHI::EmptyCompileFunction<OffscreenScope>(), &OffscreenFunctions::Execute));
        }
        return *m_producers[index];
    }

    void NoesisPass::FrameBeginInternal(FramePrepareParams params)
    {
        m_tape.Reset();
        m_onscreenDraws.clear();
        m_onscreenScopes.clear();
        m_producerCount = 0;
        m_viewport = params.m_viewportState;

        RPI::RenderPipeline* pipeline = GetRenderPipeline();
        RPI::Scene* scene = pipeline ? pipeline->GetScene() : nullptr;
        NoesisFeatureProcessor* featureProcessor = scene ? scene->GetFeatureProcessor<NoesisFeatureProcessor>() : nullptr;
        const u32 width = static_cast<u32>(AZStd::max(0.0f, m_viewport.m_maxX - m_viewport.m_minX));
        const u32 height = static_cast<u32>(AZStd::max(0.0f, m_viewport.m_maxY - m_viewport.m_minY));

        bool hasOffscreenWork = false;
        if (featureProcessor && scene->GetDefaultRenderPipeline().get() == pipeline && width > 0 && height > 0)
        {
            // Locked from the empty-views check through EndFrame(): AddView/RemoveView mutate GetViews() under
            // NoesisFeatureProcessor::GetUpdateMutex() from other threads, and BeginFrame/Render/EndFrame touch
            // the same device and views. Released before scope-producer import and Base::FrameBeginInternal below,
            // neither of which touches feature-processor state.
            AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
            if (!featureProcessor->GetViews().empty())
            {
                if (HotReload* hotReload = AZ::Interface<HotReload>::Get())
                {
                    hotReload->Flush();
                }

                featureProcessor->SetOutputSize(width, height);

                const RHI::RenderAttachmentConfiguration config = GetRenderAttachmentConfiguration();
                OutputConfig onscreen;
                onscreen.m_colorFormat = config.GetRenderTargetFormat(0);
                onscreen.m_depthStencilFormat = config.GetDepthStencilFormat();
                onscreen.m_samples = static_cast<u8>(GetMultisampleState().m_samples);

                const double seconds = AZ::TimeUsToSecondsDouble(AZ::Interface<AZ::ITime>::Get()->GetElapsedTimeUs());
                AtomRenderDevice& device = featureProcessor->GetRenderDevice();
                device.BeginFrame(m_tape, onscreen, width, height);
                // All offscreen work first so every offscreen scope precedes the onscreen scope in the frame graph.
                for (const NoesisFeatureProcessor::ViewEntry& entry : featureProcessor->GetViews())
                {
                    if (!entry.m_visible)
                    {
                        continue;
                    }
                    entry.m_view->SetSize(width, height);
                    entry.m_view->Update(seconds);
                    entry.m_view->GetRenderer()->UpdateRenderTree();
                    entry.m_view->GetRenderer()->RenderOffscreen();
                }
                for (const NoesisFeatureProcessor::ViewEntry& entry : featureProcessor->GetViews())
                {
                    if (!entry.m_visible)
                    {
                        continue;
                    }
                    entry.m_view->GetRenderer()->Render();
                }
                device.EndFrame();
                hasOffscreenWork = true;
            }
        }

        if (hasOffscreenWork)
        {
            const auto& scopes = m_tape.GetScopes();
            for (u32 i = 0; i < scopes.size(); ++i)
            {
                if (scopes[i].m_target)
                {
                    Producer& producer = AcquireProducer(m_producerCount++);
                    producer.GetUserData() = OffscreenScope{ this, i };
                    params.m_frameGraphBuilder->ImportScopeProducer(producer);
                }
                else
                {
                    m_onscreenScopes.push_back(i);
                    for (u32 d = 0; d < scopes[i].m_drawCount; ++d)
                    {
                        m_onscreenDraws.push_back(scopes[i].m_firstDraw + d);
                    }
                }
            }
        }

        Base::FrameBeginInternal(params);
    }

    void NoesisPass::Import(RHI::FrameGraphInterface& frameGraph, RPI::AttachmentImage* image)
    {
        RHI::FrameGraphAttachmentInterface database = frameGraph.GetAttachmentDatabase();
        if (!database.IsAttachmentValid(image->GetAttachmentId()))
        {
            database.ImportImage(image->GetAttachmentId(), RHI::Ptr<RHI::Image>(image->GetRHIImage()));
        }
    }

    void NoesisPass::DeclareSampled(RHI::FrameGraphInterface& frameGraph, const AZStd::vector<const AtomTexture*>& textures)
    {
        for (const AtomTexture* texture : textures)
        {
            RPI::AttachmentImage* image = texture->GetAttachmentImage();
            Import(frameGraph, image);
            frameGraph.UseShaderAttachment(
                RHI::ImageScopeAttachmentDescriptor(image->GetAttachmentId()), RHI::ScopeAttachmentAccess::Read,
                RHI::ScopeAttachmentStage::FragmentShader);
        }
    }

    void NoesisPass::OffscreenFunctions::Prepare(RHI::FrameGraphInterface frameGraph, OffscreenScope& scope)
    {
        const auto& tapeScope = scope.m_pass->m_tape.GetScopes()[scope.m_tapeScope];
        AtomRenderTarget& target = *tapeScope.m_target;

        RPI::AttachmentImage* color = target.GetColor().GetAttachmentImage();
        Import(frameGraph, color);
        // Noesis requires the previous content to be discarded, not cleared.
        RHI::AttachmentLoadStoreAction colorAction;
        colorAction.m_loadAction = RHI::AttachmentLoadAction::DontCare;
        colorAction.m_storeAction = RHI::AttachmentStoreAction::Store;
        frameGraph.UseColorAttachment(RHI::ImageScopeAttachmentDescriptor(color->GetAttachmentId(), RHI::ImageViewDescriptor(), colorAction));

        if (RPI::AttachmentImage* stencil = target.GetStencil())
        {
            Import(frameGraph, stencil);
            RHI::AttachmentLoadStoreAction stencilAction;
            stencilAction.m_clearValue = RHI::ClearValue::CreateDepthStencil(0.0f, 0);
            stencilAction.m_loadAction = RHI::AttachmentLoadAction::Clear;
            stencilAction.m_loadActionStencil = RHI::AttachmentLoadAction::Clear;
            stencilAction.m_storeAction = RHI::AttachmentStoreAction::DontCare;
            stencilAction.m_storeActionStencil = RHI::AttachmentStoreAction::DontCare;
            frameGraph.UseDepthStencilAttachment(
                RHI::ImageScopeAttachmentDescriptor(stencil->GetAttachmentId(), RHI::ImageViewDescriptor(), stencilAction),
                RHI::ScopeAttachmentAccess::ReadWrite,
                RHI::ScopeAttachmentStage::EarlyFragmentTest | RHI::ScopeAttachmentStage::LateFragmentTest);
        }

        DeclareSampled(frameGraph, tapeScope.m_sampledRenderTargets);
        frameGraph.SetEstimatedItemCount(tapeScope.m_drawCount);
    }

    void NoesisPass::OffscreenFunctions::Execute(const RHI::FrameGraphExecuteContext& context, const OffscreenScope& scope)
    {
        NoesisPass& pass = *scope.m_pass;
        const auto& tapeScope = pass.m_tape.GetScopes()[scope.m_tapeScope];
        const AtomTexture& color = tapeScope.m_target->GetColor();
        context.GetCommandList()->SetViewport(RHI::Viewport(0.0f, static_cast<float>(color.GetWidth()), 0.0f, static_cast<float>(color.GetHeight())));

        const auto& range = context.GetSubmitRange();
        for (u32 i = range.m_startIndex; i < range.m_endIndex; ++i)
        {
            Submit(context, pass.m_tape.GetDraws()[tapeScope.m_firstDraw + i], i);
        }
    }

    void NoesisPass::SetupFrameGraphDependencies(RHI::FrameGraphInterface frameGraph)
    {
        Base::SetupFrameGraphDependencies(frameGraph);
        for (u32 scopeIndex : m_onscreenScopes)
        {
            DeclareSampled(frameGraph, m_tape.GetScopes()[scopeIndex].m_sampledRenderTargets);
        }
        frameGraph.SetEstimatedItemCount(static_cast<u32>(m_onscreenDraws.size()));
    }

    void NoesisPass::BuildCommandListInternal(const RHI::FrameGraphExecuteContext& context)
    {
        context.GetCommandList()->SetViewport(m_viewport);
        const auto& range = context.GetSubmitRange();
        for (u32 i = range.m_startIndex; i < range.m_endIndex; ++i)
        {
            Submit(context, m_tape.GetDraws()[m_onscreenDraws[i]], i);
        }
    }

    void NoesisPass::Submit(const RHI::FrameGraphExecuteContext& context, const TapeDraw& draw, u32 submitIndex)
    {
        const int deviceIndex = context.GetDeviceIndex();
        RHI::DeviceDrawItem item;
        item.m_drawInstanceArgs = RHI::DrawInstanceArguments(1, 0);
        // RHI::GeometryView::GetDeviceGeometryView() is not const-qualified in the 26.05 headers (see
        // Atom/RHI/GeometryView.h and DrawItem.h::SetGeometryView, which likewise takes a non-const GeometryView*);
        // the tape's draws are otherwise read-only here, so cast away constness for this single lookup rather than
        // widen Submit's/RenderTape's interfaces to non-const for every field.
        item.m_geometryView = const_cast<RHI::GeometryView&>(draw.m_geometry).GetDeviceGeometryView(deviceIndex);
        item.m_streamIndices = draw.m_geometry.GetFullStreamBufferIndices();
        item.m_pipelineState = draw.m_pipelineState->GetDevicePipelineState(deviceIndex).get();
        item.m_uniqueShaderResourceGroup = draw.m_srg->GetRHIShaderResourceGroup()->GetDeviceShaderResourceGroup(deviceIndex).get();
        item.m_stencilRef = draw.m_stencilRef;
        item.m_scissorsCount = 1;
        item.m_scissors = &draw.m_scissor;
        context.GetCommandList()->Submit(item, submitIndex);
    }
}
