#pragma once

#include <Atom/RHI.Reflect/Base.h>
#include <Atom/RHI.Reflect/Format.h>
#include <Atom/RHI/PipelineState.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/hash.h>

#include <NsRender/RenderDevice.h>

namespace NoesisGUI
{
    struct OutputConfig
    {
        AZ::RHI::Format m_colorFormat = AZ::RHI::Format::Unknown;
        AZ::RHI::Format m_depthStencilFormat = AZ::RHI::Format::Unknown;
        AZ::u8 m_samples = 1;
    };

    class PipelineCache
    {
    public:
        bool Init();
        void Shutdown();
        const AZ::Data::Instance<AZ::RPI::Shader>& GetShader(Noesis::Shader::Enum shader) const { return m_shaders[shader]; }
        const AZ::RHI::PipelineState* Get(Noesis::Shader::Enum shader, Noesis::RenderState state, const OutputConfig& output);

    private:
        struct PipelineKey
        {
            AZ::u8 m_shader = 0;
            AZ::u8 m_renderState = 0;
            AZ::RHI::Format m_colorFormat = AZ::RHI::Format::Unknown;
            AZ::RHI::Format m_depthStencilFormat = AZ::RHI::Format::Unknown;
            AZ::u8 m_samples = 1;

            bool operator==(const PipelineKey& rhs) const
            {
                return m_shader == rhs.m_shader && m_renderState == rhs.m_renderState &&
                    m_colorFormat == rhs.m_colorFormat && m_depthStencilFormat == rhs.m_depthStencilFormat &&
                    m_samples == rhs.m_samples;
            }
        };

        struct PipelineKeyHasher
        {
            size_t operator()(const PipelineKey& key) const
            {
                size_t seed = key.m_shader;
                AZStd::hash_combine(seed, key.m_renderState);
                AZStd::hash_combine(seed, static_cast<AZ::u32>(key.m_colorFormat));
                AZStd::hash_combine(seed, static_cast<AZ::u32>(key.m_depthStencilFormat));
                AZStd::hash_combine(seed, key.m_samples);
                return seed;
            }
        };

        AZStd::array<AZ::Data::Instance<AZ::RPI::Shader>, Noesis::Shader::Count> m_shaders;
        AZStd::unordered_map<PipelineKey, AZ::RHI::ConstPtr<AZ::RHI::PipelineState>, PipelineKeyHasher> m_states;
    };
}
