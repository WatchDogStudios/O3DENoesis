#include <Render/SrgCache.h>

#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <Atom/RPI.Reflect/Image/Image.h>
#include <AzCore/std/hash.h>
#include <AzCore/Name/Name.h>
#include <Render/AtomTexture.h>
#include <Render/RenderStateMapping.h>

namespace NoesisGUI
{
    namespace
    {
        constexpr const char* ConstantNames[4] = { "m_vs0", "m_vs1", "m_ps0", "m_ps1" };
        constexpr const char* TextureNames[5] = { "m_pattern", "m_ramps", "m_image", "m_glyphs", "m_shadow" };
        constexpr const char* SamplerNames[5] = { "m_patternSampler", "m_rampsSampler", "m_imageSampler", "m_glyphsSampler", "m_shadowSampler" };

        const Noesis::UniformData& Uniform(const Noesis::Batch& batch, size_t i)
        {
            return i < 2 ? batch.vertexUniforms[i] : batch.pixelUniforms[i - 2];
        }

        Noesis::Texture* TextureAt(const Noesis::Batch& batch, size_t i)
        {
            const Noesis::Texture* const textures[5] = { batch.pattern, batch.ramps, batch.image, batch.glyphs, batch.shadow };
            return const_cast<Noesis::Texture*>(textures[i]);
        }

        Noesis::SamplerState SamplerAt(const Noesis::Batch& batch, size_t i)
        {
            const Noesis::SamplerState samplers[5] = {
                batch.patternSampler, batch.rampsSampler, batch.imageSampler, batch.glyphsSampler, batch.shadowSampler
            };
            return samplers[i];
        }
    }

    size_t HashSrgKey(const SrgKey& key)
    {
        size_t seed = key.m_shader;
        for (AZ::u32 h : key.m_uniformHashes)
        {
            AZStd::hash_combine(seed, h);
        }
        for (AZ::u64 id : key.m_textureIds)
        {
            AZStd::hash_combine(seed, id);
        }
        for (AZ::u8 s : key.m_samplers)
        {
            AZStd::hash_combine(seed, s);
        }
        return seed;
    }

    SrgKey MakeSrgKey(const Noesis::Batch& batch)
    {
        SrgKey key;
        key.m_shader = batch.shader.v;
        for (size_t i = 0; i < 4; ++i)
        {
            const Noesis::UniformData& uniform = Uniform(batch, i);
            key.m_uniformHashes[i] = uniform.values ? uniform.hash : 0;
        }
        for (size_t i = 0; i < 5; ++i)
        {
            if (Noesis::Texture* texture = TextureAt(batch, i))
            {
                key.m_textureIds[i] = static_cast<AtomTexture*>(texture)->GetId();
                key.m_samplers[i] = SamplerAt(batch, i).v;
            }
        }
        return key;
    }

    AZ::RPI::ShaderResourceGroup* SrgCache::Get(const Noesis::Batch& batch, const AZ::Data::Instance<AZ::RPI::Shader>& shader)
    {
        const SrgKey key = MakeSrgKey(batch);
        auto it = m_entries.find(key);
        if (it != m_entries.end())
        {
            it->second.m_lastUsedFrame = m_frame;
            return it->second.m_srg.get();
        }

        AZ::Data::Instance<AZ::RPI::ShaderResourceGroup> srg =
            AZ::RPI::ShaderResourceGroup::Create(shader->GetAsset(), shader->GetSupervariantIndex(), AZ::Name("NoesisDrawSrg"));
        if (!srg)
        {
            return nullptr;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            const Noesis::UniformData& uniform = Uniform(batch, i);
            const AZ::RHI::ShaderInputConstantIndex index = srg->FindShaderInputConstantIndex(AZ::Name(ConstantNames[i]));
            if (uniform.values && index.IsValid())
            {
                srg->SetConstantRaw(index, uniform.values, uniform.numDwords * sizeof(uint32_t));
            }
        }

        const AZ::Data::Instance<AZ::RPI::Image>& fallback =
            AZ::RPI::ImageSystemInterface::Get()->GetSystemImage(AZ::RPI::SystemImage::Black);
        for (size_t i = 0; i < 5; ++i)
        {
            Noesis::Texture* texture = TextureAt(batch, i);
            srg->SetImage(srg->FindShaderInputImageIndex(AZ::Name(TextureNames[i])),
                texture ? static_cast<AtomTexture*>(texture)->GetImage() : fallback);
            srg->SetSampler(srg->FindShaderInputSamplerIndex(AZ::Name(SamplerNames[i])), ToSamplerState(SamplerAt(batch, i)));
        }
        srg->Compile();

        AZ::RPI::ShaderResourceGroup* result = srg.get();
        m_entries.emplace(key, Entry{ AZStd::move(srg), m_frame });
        return result;
    }

    void SrgCache::EndFrame()
    {
        ++m_frame;
        for (auto it = m_entries.begin(); it != m_entries.end();)
        {
            if (m_frame - it->second.m_lastUsedFrame > EvictAfterFrames)
            {
                it = m_entries.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void SrgCache::Clear()
    {
        m_entries.clear();
    }
}
