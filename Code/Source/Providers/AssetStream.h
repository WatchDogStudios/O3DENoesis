#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/vector.h>

#include <NsGui/Stream.h>

namespace NoesisGUI
{
    class AssetStream final : public Noesis::Stream
    {
    public:
        //! bytes must be owned by asset (or outlive the stream when asset is empty, as in tests).
        AssetStream(AZ::Data::Asset<AZ::Data::AssetData> asset, const AZStd::vector<AZ::u8>& bytes);

        void SetPosition(uint32_t pos) override;
        uint32_t GetPosition() const override { return m_position; }
        uint32_t GetLength() const override { return static_cast<uint32_t>(m_bytes.size()); }
        uint32_t Read(void* buffer, uint32_t size) override;
        const void* GetMemoryBase() const override { return m_bytes.data(); }
        void Close() override;

    private:
        AZ::Data::Asset<AZ::Data::AssetData> m_asset;
        const AZStd::vector<AZ::u8>& m_bytes;
        uint32_t m_position = 0;
    };
}
