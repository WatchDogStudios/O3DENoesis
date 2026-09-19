#include <AzTest/AzTest.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <Render/AtomTexture.h>
#include <Render/SrgCache.h>

namespace NoesisGUI::Test
{
    class SrgCacheKeyTests : public UnitTest::LeakDetectionFixture
    {
    };

    Noesis::Batch MakeBatch()
    {
        Noesis::Batch batch{};
        batch.shader.v = Noesis::Shader::Path_Pattern;
        static const uint32_t values[] = { 1, 2 };
        batch.vertexUniforms[0] = { values, 2, 0xABCD };
        return batch;
    }

    TEST_F(SrgCacheKeyTests, IdenticalBatchesProduceEqualKeys)
    {
        Noesis::Batch a = MakeBatch();
        Noesis::Batch b = MakeBatch();
        EXPECT_TRUE(MakeSrgKey(a) == MakeSrgKey(b));
        EXPECT_EQ(HashSrgKey(MakeSrgKey(a)), HashSrgKey(MakeSrgKey(b)));
    }

    TEST_F(SrgCacheKeyTests, UniformHashTextureAndSamplerChangeTheKey)
    {
        Noesis::Batch base = MakeBatch();
        Noesis::Ptr<AtomTexture> texture = *new AtomTexture({}, 4, 4, 1, true);

        Noesis::Batch uniform = base;
        uniform.vertexUniforms[0].hash = 0x1234;
        EXPECT_FALSE(MakeSrgKey(base) == MakeSrgKey(uniform));

        Noesis::Batch textured = base;
        textured.pattern = texture.GetPtr();
        EXPECT_EQ(MakeSrgKey(textured).m_textureIds[0], texture->GetId());

        Noesis::Batch sampled = textured;
        sampled.patternSampler.v = 5;
        EXPECT_FALSE(MakeSrgKey(textured) == MakeSrgKey(sampled));
    }

    TEST_F(SrgCacheKeyTests, UnusedUniformHashIsZero)
    {
        Noesis::Batch batch = MakeBatch();
        batch.vertexUniforms[0] = { nullptr, 0, 0xFFFF };
        EXPECT_EQ(MakeSrgKey(batch).m_uniformHashes[0], 0u);
    }
}
