#include <Render/AtomRenderDevice.h>

#include <Atom/RHI.Reflect/ImageDescriptor.h>
#include <Atom/RHI.Reflect/ImageSubresource.h>
#include <Atom/RHI/ImagePool.h>
#include <Atom/RHI/IndexBufferView.h>
#include <Atom/RHI/StreamBufferView.h>
#include <Atom/RPI.Public/DynamicDraw/DynamicDrawInterface.h>
#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <AzCore/std/containers/fixed_vector.h>
#include <AzCore/std/parallel/atomic.h>
#include <Render/RenderStateMapping.h>
#include <Render/ShaderTable.h>

namespace NoesisGUI
{
    using namespace AZ;

    bool AtomRenderDevice::Init()
    {
        m_caps.linearRendering = false;
        m_caps.subpixelRendering = false;
        m_caps.depthRangeZeroToOne = true;
        // Atom presents every backend with D3D clip-space conventions (Vulkan uses a flipped viewport).
        m_caps.clipSpaceYInverted = false;
        return m_pipelines.Init();
    }

    void AtomRenderDevice::Shutdown()
    {
        m_srgs.Clear();
        m_pipelines.Shutdown();
    }

    void AtomRenderDevice::BeginFrame(NoesisTape& tape, const OutputConfig& onscreen, u32 width, u32 height)
    {
        m_tape = &tape;
        m_onscreen = onscreen;
        m_onscreenWidth = width;
        m_onscreenHeight = height;
    }

    void AtomRenderDevice::EndFrame()
    {
        m_tape = nullptr;
        // Safe even though NoesisPass replays the tape's GeometryViews/SRGs later this frame: dropping our
        // RHI::Ptr<DynamicBuffer> handles only releases our reference to the wrapper, not the GPU memory —
        // DynamicBufferAllocator sub-allocates from a persistent ring buffer that stays valid until the RPI
        // frame actually advances (DynamicBufferAllocator.h), and SrgCache::EndFrame() only evicts SRGs idle
        // for EvictAfterFrames frames, not ones just used this frame.
        m_vertices = nullptr;
        m_indices = nullptr;
        m_srgs.EndFrame();
    }

    Data::Instance<RPI::AttachmentImage> AtomRenderDevice::CreateColorImage(const char* label, u32 width, u32 height, u32 levels, RHI::Format format)
    {
        RHI::ImageDescriptor descriptor = RHI::ImageDescriptor::Create2D(
            RHI::ImageBindFlags::ShaderRead | RHI::ImageBindFlags::Color, width, height, format);
        descriptor.m_mipLevels = static_cast<u16>(levels);
        static AZStd::atomic<u64> s_counter{ 0 };
        const Name name(AZStd::string::format("Noesis_%s_%llu", label ? label : "Texture", static_cast<unsigned long long>(s_counter++)));
        return RPI::AttachmentImage::Create(*RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool(), descriptor, name);
    }

    Noesis::Ptr<Noesis::RenderTarget> AtomRenderDevice::CreateRenderTarget(
        const char* label, uint32_t width, uint32_t height, uint32_t sampleCount, bool needsStencil)
    {
        // ponytail: single-sample offscreen targets; MSAA + ResolveRenderTarget land with world-space views (M3).
        AZ_WarningOnce("NoesisGUI", sampleCount <= 1, "Offscreen MSAA is not supported yet; rendering single-sample.");

        Data::Instance<RPI::AttachmentImage> colorImage = CreateColorImage(label, width, height, 1, RHI::Format::R8G8B8A8_UNORM);
        if (!colorImage)
        {
            AZ_Error("NoesisGUI", false, "CreateRenderTarget failed to create color image '%s' (%ux%u)", label ? label : "", width, height);
            return nullptr;
        }
        Noesis::Ptr<AtomTexture> color = *new AtomTexture(colorImage, width, height, 1, true);
        color->MarkRenderTarget();

        Data::Instance<RPI::AttachmentImage> stencil;
        if (needsStencil)
        {
            const RHI::ImageDescriptor descriptor = RHI::ImageDescriptor::Create2D(RHI::ImageBindFlags::DepthStencil, width, height, StencilFormat);
            static AZStd::atomic<u64> s_counter{ 0 };
            stencil = RPI::AttachmentImage::Create(*RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool(), descriptor,
                Name(AZStd::string::format("NoesisStencil_%llu", static_cast<unsigned long long>(s_counter++))));
            if (!stencil)
            {
                AZ_Error("NoesisGUI", false, "CreateRenderTarget failed to create stencil image for '%s' (%ux%u)", label ? label : "", width, height);
                return nullptr;
            }
        }
        return *new AtomRenderTarget(color, stencil);
    }

    Noesis::Ptr<Noesis::RenderTarget> AtomRenderDevice::CloneRenderTarget(const char* label, Noesis::RenderTarget* surface)
    {
        auto* source = static_cast<AtomRenderTarget*>(surface);
        const u32 width = source->GetColor().GetWidth();
        const u32 height = source->GetColor().GetHeight();
        Data::Instance<RPI::AttachmentImage> colorImage = CreateColorImage(label, width, height, 1, RHI::Format::R8G8B8A8_UNORM);
        if (!colorImage)
        {
            AZ_Error("NoesisGUI", false, "CloneRenderTarget failed to create color image '%s' (%ux%u)", label ? label : "", width, height);
            return nullptr;
        }
        Noesis::Ptr<AtomTexture> color = *new AtomTexture(colorImage, width, height, 1, true);
        color->MarkRenderTarget();
        return *new AtomRenderTarget(color, source->GetStencilInstance());
    }

    Noesis::Ptr<Noesis::Texture> AtomRenderDevice::CreateTexture(
        const char* label, uint32_t width, uint32_t height, uint32_t numLevels, Noesis::TextureFormat::Enum format, const void** data)
    {
        const RHI::Format rhiFormat = ToImageFormat(format);
        Data::Instance<RPI::AttachmentImage> image = CreateColorImage(label, width, height, numLevels, rhiFormat);
        if (!image)
        {
            return nullptr;
        }
        Noesis::Ptr<AtomTexture> texture = *new AtomTexture(image, width, height, numLevels, format == Noesis::TextureFormat::RGBA8);
        if (data)
        {
            for (u32 level = 0; level < numLevels; ++level)
            {
                UpdateTexture(texture.GetPtr(), level, 0, 0, AZStd::max(width >> level, 1u), AZStd::max(height >> level, 1u), data[level]);
            }
        }
        return texture;
    }

    void AtomRenderDevice::UpdateTexture(
        Noesis::Texture* texture, uint32_t level, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
    {
        RPI::AttachmentImage* image = static_cast<AtomTexture*>(texture)->GetAttachmentImage();
        if (!image || !data)
        {
            return;
        }
        RHI::ImageUpdateRequest request;
        request.m_image = image->GetRHIImage();
        request.m_imageSubresource = RHI::ImageSubresource{ static_cast<u16>(level), 0 };
        request.m_imageSubresourcePixelOffset = RHI::Origin(x, y, 0);
        request.m_sourceData = data;
        request.m_sourceSubresourceLayout.Init(
            RHI::MultiDevice::AllDevices,
            RHI::GetImageSubresourceLayout(RHI::Size(width, height, 1), image->GetRHIImage()->GetDescriptor().m_format));
        const RHI::ResultCode result = image->UpdateImageContents(request);
        AZ_Error("NoesisGUI", result == RHI::ResultCode::Success, "UpdateTexture failed (%ux%u at %u,%u level %u)", width, height, x, y, level);
    }

    void AtomRenderDevice::BeginOnscreenRender()
    {
        if (!m_tape)
        {
            return;
        }
        m_tape->SetTarget(nullptr);
        m_output = m_onscreen;
        m_targetWidth = m_onscreenWidth;
        m_targetHeight = m_onscreenHeight;
        SetFullScissor();
    }

    void AtomRenderDevice::SetRenderTarget(Noesis::RenderTarget* surface)
    {
        if (!m_tape)
        {
            return;
        }
        auto* target = static_cast<AtomRenderTarget*>(surface);
        m_tape->SetTarget(target);
        m_output.m_colorFormat = RHI::Format::R8G8B8A8_UNORM;
        m_output.m_depthStencilFormat = target->GetStencil() ? StencilFormat : RHI::Format::Unknown;
        m_output.m_samples = 1;
        m_targetWidth = target->GetColor().GetWidth();
        m_targetHeight = target->GetColor().GetHeight();
        SetFullScissor();
    }

    void AtomRenderDevice::BeginTile(Noesis::RenderTarget*, const Noesis::Tile& tile)
    {
        // Noesis tiles have a lower-left origin; RHI scissors are top-left.
        const s32 top = static_cast<s32>(m_targetHeight) - static_cast<s32>(tile.y + tile.height);
        m_scissor = RHI::Scissor(tile.x, top, tile.x + tile.width, top + tile.height);
    }

    void AtomRenderDevice::EndTile(Noesis::RenderTarget*)
    {
        SetFullScissor();
    }

    void AtomRenderDevice::SetFullScissor()
    {
        m_scissor = RHI::Scissor(0, 0, m_targetWidth, m_targetHeight);
    }

    void* AtomRenderDevice::Map(RHI::Ptr<RPI::DynamicBuffer>& buffer, u32 bytes)
    {
        buffer = RPI::DynamicDrawInterface::Get()->GetDynamicBuffer(bytes, RHI::Alignment::InputAssembly);
        if (!buffer || buffer->GetBufferAddress().empty())
        {
            AZ_WarningOnce("NoesisGUI", false, "Out of dynamic draw memory; skipping Noesis geometry this frame.");
            buffer = nullptr;
            m_scratch.resize(bytes);
            return m_scratch.data();
        }
        return buffer->GetBufferAddress().begin()->second;
    }

    void AtomRenderDevice::Unmap(const RHI::Ptr<RPI::DynamicBuffer>& buffer)
    {
        if (!buffer)
        {
            return;
        }
        const auto& addresses = buffer->GetBufferAddress();
        const void* source = addresses.begin()->second;
        for (const auto& [deviceIndex, address] : addresses)
        {
            if (address != source)
            {
                memcpy(address, source, buffer->GetSize());
            }
        }
    }

    void* AtomRenderDevice::MapVertices(uint32_t bytes) { return Map(m_vertices, bytes); }
    void AtomRenderDevice::UnmapVertices() { Unmap(m_vertices); }
    void* AtomRenderDevice::MapIndices(uint32_t bytes) { return Map(m_indices, bytes); }
    void AtomRenderDevice::UnmapIndices() { Unmap(m_indices); }

    void AtomRenderDevice::DrawBatch(const Noesis::Batch& batch)
    {
        if (!m_tape || !m_vertices || !m_indices)
        {
            return;
        }
        const auto shader = static_cast<Noesis::Shader::Enum>(batch.shader.v);
        if (batch.pixelShader || !ShaderTable::IsSupported(shader))
        {
            AZ_WarningOnce("NoesisGUI", false, "Custom and subpixel Noesis shaders are not supported yet; batch skipped.");
            return;
        }

        const RHI::PipelineState* pipelineState = m_pipelines.Get(shader, batch.renderState, m_output);
        RPI::ShaderResourceGroup* srg = pipelineState ? m_srgs.Get(batch, m_pipelines.GetShader(shader)) : nullptr;
        if (!srg)
        {
            return;
        }

        const u32 stride = ShaderTable::GetVertexStride(shader);
        const RHI::StreamBufferView vertices = m_vertices->GetStreamBufferView(stride);

        TapeDraw draw;
        draw.m_pipelineState = pipelineState;
        draw.m_srg = srg;
        draw.m_scissor = m_scissor;
        draw.m_stencilRef = batch.stencilRef;
        draw.m_geometry.SetDrawArguments(RHI::DrawIndexed(0, batch.numIndices, batch.startIndex));
        draw.m_geometry.SetIndexBufferView(m_indices->GetIndexBufferView(RHI::IndexFormat::Uint16));
        draw.m_geometry.AddStreamBufferView(
            RHI::StreamBufferView(*vertices.GetBuffer(), vertices.GetByteOffset() + batch.vertexOffset, batch.numVertices * stride, stride));

        AZStd::fixed_vector<const AtomTexture*, 5> sampled;
        for (Noesis::Texture* texture : { batch.pattern, batch.ramps, batch.image, batch.glyphs, batch.shadow })
        {
            if (texture && static_cast<AtomTexture*>(texture)->IsRenderTarget())
            {
                sampled.push_back(static_cast<AtomTexture*>(texture));
            }
        }
        m_tape->AddDraw(AZStd::move(draw), AZStd::span<const AtomTexture* const>(sampled.data(), sampled.size()));
    }
}
