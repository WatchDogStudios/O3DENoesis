#pragma once

#include <Atom/RHI.Reflect/InputStreamLayout.h>
#include <AzCore/std/string/string.h>

#include <NsRender/RenderDevice.h>

namespace NoesisGUI::ShaderTable
{
    const char* GetName(Noesis::Shader::Enum shader);
    bool IsSupported(Noesis::Shader::Enum shader);
    AZStd::string GetProductPath(Noesis::Shader::Enum shader);
    AZ::RHI::InputStreamLayout BuildInputStreamLayout(Noesis::Shader::Enum shader);
    AZ::u32 GetVertexStride(Noesis::Shader::Enum shader);
}
