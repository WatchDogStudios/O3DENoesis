#pragma once

#include <Atom/RHI/GeometryView.h>
#include <Atom/RHI.Reflect/Scissor.h>
#include <Atom/RPI.Public/DynamicDraw/DynamicBuffer.h>
#include <Render/AtomTexture.h>
#include <Render/PipelineCache.h>
#include <Render/RenderTape.h>
#include <Render/SrgCache.h>

#include <NsRender/RenderDevice.h>

namespace NoesisGUI
{
    struct TapeDraw
    {
        AZ::RHI::GeometryView m_geometry{ AZ::RHI::MultiDevice::AllDevices };
        const AZ::RHI::PipelineState* m_pipelineState = nullptr;
        AZ::RPI::ShaderResourceGroup* m_srg = nullptr;
        AZ::RHI::Scissor m_scissor;
        AZ::u8 m_stencilRef = 0;
    };

    using NoesisTape = RenderTape<TapeDraw, AtomRenderTarget, AtomTexture>;

    class AtomRenderDevice final : public Noesis::RenderDevice
    {
    public:
        bool Init();
        void Shutdown();
        //! Everything Noesis draws until EndFrame() is recorded into tape. onscreen describes the pass output.
        void BeginFrame(NoesisTape& tape, const OutputConfig& onscreen, AZ::u32 width, AZ::u32 height);
        void EndFrame();

    private:
        const Noesis::DeviceCaps& GetCaps() const override { return m_caps; }
        Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(
            const char* label, uint32_t width, uint32_t height, uint32_t sampleCount, bool needsStencil) override;
        Noesis::Ptr<Noesis::RenderTarget> CloneRenderTarget(const char* label, Noesis::RenderTarget* surface) override;
        Noesis::Ptr<Noesis::Texture> CreateTexture(
            const char* label, uint32_t width, uint32_t height, uint32_t numLevels, Noesis::TextureFormat::Enum format,
            const void** data) override;
        void UpdateTexture(Noesis::Texture* texture, uint32_t level, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
            const void* data) override;
        void BeginOffscreenRender() override {}
        void EndOffscreenRender() override {}
        void BeginOnscreenRender() override;
        void EndOnscreenRender() override {}
        void SetRenderTarget(Noesis::RenderTarget* surface) override;
        void BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile) override;
        void EndTile(Noesis::RenderTarget* surface) override;
        void ResolveRenderTarget(Noesis::RenderTarget*, const Noesis::Tile*, uint32_t) override {}
        void* MapVertices(uint32_t bytes) override;
        void UnmapVertices() override;
        void* MapIndices(uint32_t bytes) override;
        void UnmapIndices() override;
        void DrawBatch(const Noesis::Batch& batch) override;

        AZ::Data::Instance<AZ::RPI::AttachmentImage> CreateColorImage(const char* label, AZ::u32 width, AZ::u32 height, AZ::u32 levels, AZ::RHI::Format format);
        void* Map(AZ::RHI::Ptr<AZ::RPI::DynamicBuffer>& buffer, AZ::u32 bytes);
        void Unmap(const AZ::RHI::Ptr<AZ::RPI::DynamicBuffer>& buffer);
        void SetFullScissor();

        Noesis::DeviceCaps m_caps;
        PipelineCache m_pipelines;
        SrgCache m_srgs;

        NoesisTape* m_tape = nullptr;
        OutputConfig m_onscreen;
        AZ::u32 m_onscreenWidth = 0;
        AZ::u32 m_onscreenHeight = 0;

        OutputConfig m_output;
        AZ::u32 m_targetWidth = 0;
        AZ::u32 m_targetHeight = 0;
        AZ::RHI::Scissor m_scissor;

        AZ::RHI::Ptr<AZ::RPI::DynamicBuffer> m_vertices;
        AZ::RHI::Ptr<AZ::RPI::DynamicBuffer> m_indices;
        AZStd::vector<AZ::u8> m_scratch;
    };
}
