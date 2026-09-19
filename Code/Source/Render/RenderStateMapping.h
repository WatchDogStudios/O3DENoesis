#pragma once

#include <Atom/RHI.Reflect/Format.h>
#include <Atom/RHI.Reflect/RenderStates.h>
#include <Atom/RHI.Reflect/SamplerState.h>

#include <NsRender/RenderDevice.h>

namespace NoesisGUI
{
    inline constexpr AZ::RHI::Format StencilFormat = AZ::RHI::Format::D32_FLOAT_S8X24_UINT;

    void ApplyRenderState(Noesis::RenderState state, AZ::RHI::RenderStates& renderStates);
    AZ::RHI::SamplerState ToSamplerState(Noesis::SamplerState state);
    AZ::RHI::Format ToImageFormat(Noesis::TextureFormat::Enum format);
}
