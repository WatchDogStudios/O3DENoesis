#include <Render/ShaderTable.h>

#include <Atom/RHI.Reflect/InputStreamLayoutBuilder.h>
#include <AzCore/Debug/Trace.h>
#include <AzCore/std/string/conversions.h>

namespace NoesisGUI::ShaderTable
{
    namespace
    {
        // Index = Noesis::Shader::Enum. nullptr = not generated (see Tools/generate_shaders.py).
        constexpr const char* Names[Noesis::Shader::Count] = {
            "RGBA", "Mask", "Clear",
            "Path_Solid", "Path_Linear", "Path_Radial", "Path_Pattern", "Path_Pattern_Clamp", "Path_Pattern_Repeat",
            "Path_Pattern_MirrorU", "Path_Pattern_MirrorV", "Path_Pattern_Mirror",
            "Path_AA_Solid", "Path_AA_Linear", "Path_AA_Radial", "Path_AA_Pattern", "Path_AA_Pattern_Clamp", "Path_AA_Pattern_Repeat",
            "Path_AA_Pattern_MirrorU", "Path_AA_Pattern_MirrorV", "Path_AA_Pattern_Mirror",
            "SDF_Solid", "SDF_Linear", "SDF_Radial", "SDF_Pattern", "SDF_Pattern_Clamp", "SDF_Pattern_Repeat",
            "SDF_Pattern_MirrorU", "SDF_Pattern_MirrorV", "SDF_Pattern_Mirror",
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            "Opacity_Solid", "Opacity_Linear", "Opacity_Radial", "Opacity_Pattern", "Opacity_Pattern_Clamp", "Opacity_Pattern_Repeat",
            "Opacity_Pattern_MirrorU", "Opacity_Pattern_MirrorV", "Opacity_Pattern_Mirror",
            "Upsample", "Downsample", "Shadow", "Blur",
            nullptr,
        };

        struct Channel
        {
            const char* m_semantic;
            AZ::RHI::Format m_format;
        };

        // Index = Noesis::Shader::Vertex::Format::Attr. Semantics match the SDK HLSL kept by the generator.
        constexpr Channel Channels[Noesis::Shader::Vertex::Format::Attr::Count] = {
            { "POSITION", AZ::RHI::Format::R32G32_FLOAT },
            { "COLOR", AZ::RHI::Format::R8G8B8A8_UNORM },
            { "TEXCOORD0", AZ::RHI::Format::R32G32_FLOAT },
            { "TEXCOORD1", AZ::RHI::Format::R32G32_FLOAT },
            { "COVERAGE", AZ::RHI::Format::R32_FLOAT },
            { "RECT", AZ::RHI::Format::R16G16B16A16_UNORM },
            { "TILE", AZ::RHI::Format::R32G32B32A32_FLOAT },
            { "IMAGE_POSITION", AZ::RHI::Format::R32G32B32A32_FLOAT },
        };

        AZ::u8 FormatOf(Noesis::Shader::Enum shader)
        {
            return Noesis::FormatForVertex[Noesis::VertexForShader[shader]];
        }
    }

    const char* GetName(Noesis::Shader::Enum shader)
    {
        return shader < Noesis::Shader::Count ? Names[shader] : nullptr;
    }

    bool IsSupported(Noesis::Shader::Enum shader)
    {
        return GetName(shader) != nullptr;
    }

    AZStd::string GetProductPath(Noesis::Shader::Enum shader)
    {
        AZ_Assert(IsSupported(shader), "No generated Atom shader for Noesis shader %d", static_cast<int>(shader));
        AZStd::string name = GetName(shader);
        AZStd::to_lower(name.begin(), name.end());
        return AZStd::string::format("shaders/noesisgui/generated/%s.azshader", name.c_str());
    }

    AZ::RHI::InputStreamLayout BuildInputStreamLayout(Noesis::Shader::Enum shader)
    {
        AZ::RHI::InputStreamLayoutBuilder builder;
        builder.SetTopology(AZ::RHI::PrimitiveTopology::TriangleList);
        auto* buffer = builder.AddBuffer();
        const AZ::u8 attributes = Noesis::AttributesForFormat[FormatOf(shader)];
        for (AZ::u8 attr = 0; attr < Noesis::Shader::Vertex::Format::Attr::Count; ++attr)
        {
            if (attributes & (1 << attr))
            {
                buffer->Channel(Channels[attr].m_semantic, Channels[attr].m_format);
            }
        }
        return builder.End();
    }

    AZ::u32 GetVertexStride(Noesis::Shader::Enum shader)
    {
        return Noesis::SizeForFormat[FormatOf(shader)];
    }
}
