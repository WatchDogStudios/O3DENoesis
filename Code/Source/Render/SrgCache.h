#pragma once

#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/containers/unordered_map.h>

#include <NsRender/RenderDevice.h>

namespace NoesisGUI
{
    struct SrgKey
    {
        AZ::u8 m_shader = 0;
        AZStd::array<AZ::u32, 4> m_uniformHashes{};
        AZStd::array<AZ::u64, 5> m_textureIds{};
        AZStd::array<AZ::u8, 5> m_samplers{};

        bool operator==(const SrgKey& rhs) const
        {
            return m_shader == rhs.m_shader && m_uniformHashes == rhs.m_uniformHashes && m_textureIds == rhs.m_textureIds &&
                m_samplers == rhs.m_samplers;
        }
    };

    size_t HashSrgKey(const SrgKey& key);
    SrgKey MakeSrgKey(const Noesis::Batch& batch);

    class SrgCache
    {
    public:
        static constexpr AZ::u32 EvictAfterFrames = 120;

        AZ::RPI::ShaderResourceGroup* Get(const Noesis::Batch& batch, const AZ::Data::Instance<AZ::RPI::Shader>& shader);
        void EndFrame();
        void Clear();

    private:
        struct KeyHasher
        {
            size_t operator()(const SrgKey& key) const { return HashSrgKey(key); }
        };

        struct Entry
        {
            AZ::Data::Instance<AZ::RPI::ShaderResourceGroup> m_srg;
            AZ::u64 m_lastUsedFrame = 0;
        };

        // ponytail: entries keep their images alive up to EvictAfterFrames after the texture dies; key on a
        // texture-destroyed callback if that memory ever matters.
        AZStd::unordered_map<SrgKey, Entry, KeyHasher> m_entries;
        AZ::u64 m_frame = 0;
    };
}
