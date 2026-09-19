#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/vector.h>

namespace AZ
{
    class ReflectContext;
}

namespace NoesisGUI
{
    class NoesisXamlAsset final : public AZ::Data::AssetData
    {
    public:
        static constexpr inline const char* DisplayName = "NoesisXamlAsset";
        static constexpr inline const char* Extension = "noesisxaml";
        static constexpr inline const char* Group = "UI";

        AZ_RTTI(NoesisXamlAsset, "{4E8B2C17-9A3D-4F61-8B5E-0C7D6A1F2E93}", AZ::Data::AssetData);
        AZ_CLASS_ALLOCATOR(NoesisXamlAsset, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);

        AZStd::vector<AZ::u8> m_data;
    };
}
