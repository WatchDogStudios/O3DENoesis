#include <Render/RenderStateMapping.h>

namespace NoesisGUI
{
    using namespace AZ::RHI;

    namespace
    {
        void SetBlend(TargetBlendState& t, BlendFactor src, BlendFactor dst, BlendFactor srcAlpha, BlendFactor dstAlpha)
        {
            t.m_enable = 1;
            t.m_blendSource = src;
            t.m_blendDest = dst;
            t.m_blendAlphaSource = srcAlpha;
            t.m_blendAlphaDest = dstAlpha;
        }

        void SetStencil(StencilState& s, bool enable, ComparisonFunc func, StencilOp passOp)
        {
            s.m_enable = enable ? 1 : 0;
            s.m_readMask = 0xFF;
            s.m_writeMask = 0xFF;
            for (StencilOpState* face : { &s.m_frontFace, &s.m_backFace })
            {
                face->m_failOp = StencilOp::Keep;
                face->m_depthFailOp = StencilOp::Keep;
                face->m_func = func;
                face->m_passOp = passOp;
            }
        }
    }

    void ApplyRenderState(Noesis::RenderState state, RenderStates& renderStates)
    {
        renderStates.m_rasterState.m_fillMode = state.f.wireframe ? FillMode::Wireframe : FillMode::Solid;
        renderStates.m_rasterState.m_cullMode = CullMode::None;

        TargetBlendState& target = renderStates.m_blendState.m_targets[0];
        target = TargetBlendState();
        if (!state.f.colorEnable)
        {
            target.m_writeMask = 0;
        }
        else
        {
            target.m_writeMask = 0xF;
            switch (state.f.blendMode)
            {
            case Noesis::BlendMode::Src:
                break;
            case Noesis::BlendMode::SrcOver:
                SetBlend(target, BlendFactor::One, BlendFactor::AlphaSourceInverse, BlendFactor::One, BlendFactor::AlphaSourceInverse);
                break;
            case Noesis::BlendMode::SrcOver_Multiply:
                SetBlend(target, BlendFactor::ColorDest, BlendFactor::AlphaSourceInverse, BlendFactor::One, BlendFactor::AlphaSourceInverse);
                break;
            case Noesis::BlendMode::SrcOver_Screen:
                SetBlend(target, BlendFactor::One, BlendFactor::ColorSourceInverse, BlendFactor::One, BlendFactor::AlphaSourceInverse);
                break;
            case Noesis::BlendMode::SrcOver_Additive:
                SetBlend(target, BlendFactor::One, BlendFactor::One, BlendFactor::One, BlendFactor::AlphaSourceInverse);
                break;
            case Noesis::BlendMode::SrcOver_Dual:
                SetBlend(target, BlendFactor::One, BlendFactor::ColorSource1Inverse, BlendFactor::One, BlendFactor::AlphaSource1Inverse);
                break;
            }
        }

        DepthStencilState& ds = renderStates.m_depthStencilState;
        ds.m_depth.m_writeMask = DepthWriteMask::Zero;
        ds.m_depth.m_enable = 0;
        ds.m_depth.m_func = ComparisonFunc::Never;
        switch (state.f.stencilMode)
        {
        case Noesis::StencilMode::Disabled:
            SetStencil(ds.m_stencil, false, ComparisonFunc::Equal, StencilOp::Keep);
            break;
        case Noesis::StencilMode::Equal_Keep:
            SetStencil(ds.m_stencil, true, ComparisonFunc::Equal, StencilOp::Keep);
            break;
        case Noesis::StencilMode::Equal_Incr:
            SetStencil(ds.m_stencil, true, ComparisonFunc::Equal, StencilOp::Increment);
            break;
        case Noesis::StencilMode::Equal_Decr:
            SetStencil(ds.m_stencil, true, ComparisonFunc::Equal, StencilOp::Decrement);
            break;
        case Noesis::StencilMode::Clear:
            SetStencil(ds.m_stencil, true, ComparisonFunc::Always, StencilOp::Zero);
            break;
        case Noesis::StencilMode::Disabled_ZTest:
            ds.m_depth.m_enable = 1;
            ds.m_depth.m_func = ComparisonFunc::GreaterEqual;
            SetStencil(ds.m_stencil, false, ComparisonFunc::Equal, StencilOp::Keep);
            break;
        case Noesis::StencilMode::Equal_Keep_ZTest:
            ds.m_depth.m_enable = 1;
            ds.m_depth.m_func = ComparisonFunc::GreaterEqual;
            SetStencil(ds.m_stencil, true, ComparisonFunc::Equal, StencilOp::Keep);
            break;
        }
    }

    SamplerState ToSamplerState(Noesis::SamplerState state)
    {
        SamplerState out;
        const bool linear = state.f.minmagFilter == Noesis::MinMagFilter::Linear;
        out.m_filterMin = linear ? FilterMode::Linear : FilterMode::Point;
        out.m_filterMag = out.m_filterMin;
        out.m_filterMip = state.f.mipFilter == Noesis::MipFilter::Linear ? FilterMode::Linear : FilterMode::Point;
        if (state.f.mipFilter == Noesis::MipFilter::Disabled)
        {
            out.m_mipLodMax = 0.0f;
        }

        AddressMode u = AddressMode::Clamp;
        AddressMode v = AddressMode::Clamp;
        switch (state.f.wrapMode)
        {
        case Noesis::WrapMode::ClampToEdge:
            break;
        case Noesis::WrapMode::ClampToZero:
            u = v = AddressMode::Border;
            out.m_borderColor = BorderColor::TransparentBlack;
            break;
        case Noesis::WrapMode::Repeat:
            u = v = AddressMode::Wrap;
            break;
        case Noesis::WrapMode::MirrorU:
            u = AddressMode::Mirror;
            v = AddressMode::Wrap;
            break;
        case Noesis::WrapMode::MirrorV:
            u = AddressMode::Wrap;
            v = AddressMode::Mirror;
            break;
        case Noesis::WrapMode::Mirror:
            u = v = AddressMode::Mirror;
            break;
        }
        out.m_addressU = u;
        out.m_addressV = v;
        out.m_addressW = AddressMode::Clamp;
        return out;
    }

    Format ToImageFormat(Noesis::TextureFormat::Enum format)
    {
        // Screen UI is composited post-tonemap, so DeviceCaps::linearRendering is false and textures are UNORM.
        return format == Noesis::TextureFormat::R8 ? Format::R8_UNORM : Format::R8G8B8A8_UNORM;
    }
}
