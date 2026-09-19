#include <Render/PipelineCache.h>

#include <Atom/RHI.Reflect/RenderAttachmentLayoutBuilder.h>
#include <Atom/RHI/PipelineStateDescriptor.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <AzCore/Debug/Trace.h>
#include <Render/RenderStateMapping.h>
#include <Render/ShaderTable.h>

namespace NoesisGUI
{
    bool PipelineCache::Init()
    {
        bool allLoaded = true;
        for (int i = 0; i < Noesis::Shader::Count; ++i)
        {
            const auto shader = static_cast<Noesis::Shader::Enum>(i);
            if (ShaderTable::IsSupported(shader))
            {
                m_shaders[i] = AZ::RPI::LoadCriticalShader(ShaderTable::GetProductPath(shader));
                allLoaded &= m_shaders[i] != nullptr;
            }
        }
        return allLoaded;
    }

    void PipelineCache::Shutdown()
    {
        m_states.clear();
        m_shaders = {};
    }

    const AZ::RHI::PipelineState* PipelineCache::Get(Noesis::Shader::Enum shader, Noesis::RenderState state, const OutputConfig& output)
    {
        const AZ::Data::Instance<AZ::RPI::Shader>& rpiShader = m_shaders[shader];
        if (!rpiShader)
        {
            return nullptr;
        }

        const PipelineKey key{ static_cast<AZ::u8>(shader), state.v, output.m_colorFormat, output.m_depthStencilFormat, output.m_samples };
        if (auto it = m_states.find(key); it != m_states.end())
        {
            return it->second.get();
        }

        AZ::RHI::PipelineStateDescriptorForDraw descriptor;
        rpiShader->GetRootVariant().ConfigurePipelineState(descriptor);
        descriptor.m_inputStreamLayout = ShaderTable::BuildInputStreamLayout(shader);

        AZ::RHI::RenderAttachmentLayoutBuilder builder;
        auto* subpass = builder.AddSubpass();
        subpass->RenderTargetAttachment(output.m_colorFormat);
        if (output.m_depthStencilFormat != AZ::RHI::Format::Unknown)
        {
            subpass->DepthStencilAttachment(output.m_depthStencilFormat);
        }
        builder.End(descriptor.m_renderAttachmentConfiguration.m_renderAttachmentLayout);
        descriptor.m_renderAttachmentConfiguration.m_subpassIndex = 0;

        descriptor.m_renderStates.m_multisampleState.m_samples = output.m_samples;
        ApplyRenderState(state, descriptor.m_renderStates);

        const AZ::RHI::PipelineState* pipelineState = rpiShader->AcquirePipelineState(descriptor);
        AZ_ErrorOnce("NoesisGUI", pipelineState, "Failed to acquire pipeline state for shader %s", ShaderTable::GetName(shader));
        if (pipelineState)
        {
            m_states.emplace(key, pipelineState);
        }
        return pipelineState;
    }
}
