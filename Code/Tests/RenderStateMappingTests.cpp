#include <AzTest/AzTest.h>
#include <AzCore/UnitTest/TestTypes.h>

#include <Render/RenderStateMapping.h>

namespace NoesisGUI::Test
{
    using namespace AZ::RHI;

    Noesis::RenderState MakeState(bool color, Noesis::BlendMode::Enum blend, Noesis::StencilMode::Enum stencil)
    {
        Noesis::RenderState state;
        state.v = 0;
        state.f.colorEnable = color ? 1 : 0;
        state.f.blendMode = blend;
        state.f.stencilMode = stencil;
        return state;
    }

    class RenderStateMappingTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(RenderStateMappingTests, SrcOver_IsPremultipliedAlpha)
    {
        RenderStates rs;
        ApplyRenderState(MakeState(true, Noesis::BlendMode::SrcOver, Noesis::StencilMode::Disabled), rs);
        const TargetBlendState& t = rs.m_blendState.m_targets[0];
        EXPECT_EQ(t.m_enable, 1u);
        EXPECT_EQ(t.m_writeMask, 0xFu);
        EXPECT_EQ(t.m_blendSource, BlendFactor::One);
        EXPECT_EQ(t.m_blendDest, BlendFactor::AlphaSourceInverse);
        EXPECT_EQ(t.m_blendAlphaSource, BlendFactor::One);
        EXPECT_EQ(t.m_blendAlphaDest, BlendFactor::AlphaSourceInverse);
    }

    TEST_F(RenderStateMappingTests, Multiply_Screen_Additive)
    {
        RenderStates rs;
        ApplyRenderState(MakeState(true, Noesis::BlendMode::SrcOver_Multiply, Noesis::StencilMode::Disabled), rs);
        EXPECT_EQ(rs.m_blendState.m_targets[0].m_blendSource, BlendFactor::ColorDest);
        ApplyRenderState(MakeState(true, Noesis::BlendMode::SrcOver_Screen, Noesis::StencilMode::Disabled), rs);
        EXPECT_EQ(rs.m_blendState.m_targets[0].m_blendDest, BlendFactor::ColorSourceInverse);
        ApplyRenderState(MakeState(true, Noesis::BlendMode::SrcOver_Additive, Noesis::StencilMode::Disabled), rs);
        EXPECT_EQ(rs.m_blendState.m_targets[0].m_blendDest, BlendFactor::One);
        EXPECT_EQ(rs.m_blendState.m_targets[0].m_blendAlphaDest, BlendFactor::AlphaSourceInverse);
    }

    TEST_F(RenderStateMappingTests, ColorDisabled_WritesNothing)
    {
        RenderStates rs;
        ApplyRenderState(MakeState(false, Noesis::BlendMode::SrcOver, Noesis::StencilMode::Equal_Incr), rs);
        EXPECT_EQ(rs.m_blendState.m_targets[0].m_enable, 0u);
        EXPECT_EQ(rs.m_blendState.m_targets[0].m_writeMask, 0u);
    }

    TEST_F(RenderStateMappingTests, StencilModes)
    {
        RenderStates rs;
        ApplyRenderState(MakeState(false, Noesis::BlendMode::Src, Noesis::StencilMode::Equal_Incr), rs);
        EXPECT_EQ(rs.m_depthStencilState.m_stencil.m_enable, 1u);
        EXPECT_EQ(rs.m_depthStencilState.m_stencil.m_frontFace.m_func, ComparisonFunc::Equal);
        EXPECT_EQ(rs.m_depthStencilState.m_stencil.m_frontFace.m_passOp, StencilOp::Increment);
        EXPECT_EQ(rs.m_depthStencilState.m_depth.m_enable, 0u);

        ApplyRenderState(MakeState(false, Noesis::BlendMode::Src, Noesis::StencilMode::Clear), rs);
        EXPECT_EQ(rs.m_depthStencilState.m_stencil.m_backFace.m_func, ComparisonFunc::Always);
        EXPECT_EQ(rs.m_depthStencilState.m_stencil.m_backFace.m_passOp, StencilOp::Zero);

        ApplyRenderState(MakeState(true, Noesis::BlendMode::Src, Noesis::StencilMode::Equal_Keep_ZTest), rs);
        EXPECT_EQ(rs.m_depthStencilState.m_depth.m_enable, 1u);
        EXPECT_EQ(rs.m_depthStencilState.m_depth.m_func, ComparisonFunc::GreaterEqual);
        EXPECT_EQ(rs.m_depthStencilState.m_depth.m_writeMask, DepthWriteMask::Zero);
    }

    TEST_F(RenderStateMappingTests, Wireframe)
    {
        RenderStates rs;
        Noesis::RenderState state = MakeState(true, Noesis::BlendMode::Src, Noesis::StencilMode::Disabled);
        state.f.wireframe = 1;
        ApplyRenderState(state, rs);
        EXPECT_EQ(rs.m_rasterState.m_fillMode, FillMode::Wireframe);
    }

    TEST_F(RenderStateMappingTests, Samplers)
    {
        Noesis::SamplerState s;
        s.v = 0;
        s.f.wrapMode = Noesis::WrapMode::ClampToZero;
        s.f.minmagFilter = Noesis::MinMagFilter::Linear;
        s.f.mipFilter = Noesis::MipFilter::Disabled;
        SamplerState out = ToSamplerState(s);
        EXPECT_EQ(out.m_addressU, AddressMode::Border);
        EXPECT_EQ(out.m_borderColor, BorderColor::TransparentBlack);
        EXPECT_EQ(out.m_filterMin, FilterMode::Linear);
        EXPECT_FLOAT_EQ(out.m_mipLodMax, 0.0f);

        s.f.wrapMode = Noesis::WrapMode::MirrorU;
        s.f.mipFilter = Noesis::MipFilter::Linear;
        out = ToSamplerState(s);
        EXPECT_EQ(out.m_addressU, AddressMode::Mirror);
        EXPECT_EQ(out.m_addressV, AddressMode::Wrap);
        EXPECT_EQ(out.m_filterMip, FilterMode::Linear);
    }

    TEST_F(RenderStateMappingTests, TextureFormats)
    {
        EXPECT_EQ(ToImageFormat(Noesis::TextureFormat::RGBA8), Format::R8G8B8A8_UNORM);
        EXPECT_EQ(ToImageFormat(Noesis::TextureFormat::RGBX8), Format::R8G8B8A8_UNORM);
        EXPECT_EQ(ToImageFormat(Noesis::TextureFormat::R8), Format::R8_UNORM);
    }
}
