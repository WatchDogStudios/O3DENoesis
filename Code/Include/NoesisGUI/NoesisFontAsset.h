#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/vector.h>

namespace AZ
{
    class ReflectContext;
}

namespace NoesisGUI
{
    class NoesisFontAsset final : public AZ::Data::AssetData
    {
    public:
        static constexpr inline const char* DisplayName = "NoesisFontAsset";
        static constexpr inline const char* Extension = "noesisfont";
        static constexpr inline const char* Group = "UI";

        AZ_RTTI(NoesisFontAsset, "{9D1F3B6A-2C8E-4A57-A4D0-6E2B9C8F1A74}", AZ::Data::AssetData);
        AZ_CLASS_ALLOCATOR(NoesisFontAsset, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);

        AZStd::vector<AZ::u8> m_data;
    };
}
