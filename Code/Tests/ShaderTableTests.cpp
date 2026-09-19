#include <AzTest/AzTest.h>
#include <AzCore/UnitTest/TestTypes.h>

#include <Render/ShaderTable.h>

namespace NoesisGUI::Test
{
    class ShaderTableTests : public UnitTest::LeakDetectionFixture
    {
    };

    TEST_F(ShaderTableTests, SupportedSetMatchesGenerator)
    {
        int supported = 0;
        for (int i = 0; i < Noesis::Shader::Count; ++i)
        {
            supported += ShaderTable::IsSupported(static_cast<Noesis::Shader::Enum>(i)) ? 1 : 0;
        }
        EXPECT_EQ(supported, 43);
        EXPECT_FALSE(ShaderTable::IsSupported(Noesis::Shader::SDF_LCD_Solid));
        EXPECT_FALSE(ShaderTable::IsSupported(Noesis::Shader::Custom_Effect));
    }

    TEST_F(ShaderTableTests, ProductPathIsLowercaseAzshader)
    {
        EXPECT_STREQ(ShaderTable::GetProductPath(Noesis::Shader::Path_AA_Solid).c_str(), "shaders/noesisgui/generated/path_aa_solid.azshader");
    }

    TEST_F(ShaderTableTests, LayoutStrideMatchesNoesisVertexFormat)
    {
        for (int i = 0; i < Noesis::Shader::Count; ++i)
        {
            const auto shader = static_cast<Noesis::Shader::Enum>(i);
            if (!ShaderTable::IsSupported(shader))
            {
                continue;
            }
            const AZ::u8 format = Noesis::FormatForVertex[Noesis::VertexForShader[i]];
            const AZ::RHI::InputStreamLayout layout = ShaderTable::BuildInputStreamLayout(shader);
            ASSERT_EQ(layout.GetStreamBuffers().size(), 1u) << ShaderTable::GetName(shader);
            EXPECT_EQ(layout.GetStreamBuffers()[0].m_byteStride, Noesis::SizeForFormat[format]) << ShaderTable::GetName(shader);
            EXPECT_EQ(ShaderTable::GetVertexStride(shader), Noesis::SizeForFormat[format]);
        }
    }
}
