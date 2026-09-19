#include <Providers/AssetStream.h>

#include <AzCore/std/algorithm.h>

namespace NoesisGUI
{
    AssetStream::AssetStream(AZ::Data::Asset<AZ::Data::AssetData> asset, const AZStd::vector<AZ::u8>& bytes)
        : m_asset(AZStd::move(asset))
        , m_bytes(bytes)
    {
    }

    void AssetStream::SetPosition(uint32_t pos)
    {
        m_position = AZStd::min(pos, GetLength());
    }

    uint32_t AssetStream::Read(void* buffer, uint32_t size)
    {
        const uint32_t count = AZStd::min(size, GetLength() - m_position);
        memcpy(buffer, m_bytes.data() + m_position, count);
        m_position += count;
        return count;
    }

    void AssetStream::Close()
    {
        m_asset.Reset();
    }
}
