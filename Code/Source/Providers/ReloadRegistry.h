#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/string/string.h>

namespace NoesisGUI
{
    enum class ReloadKind : AZ::u8
    {
        Xaml,
        Texture
    };

    struct ReloadChange
    {
        ReloadKind m_kind;
        AZStd::string m_uri;
    };

    class ReloadRegistry
    {
    public:
        void Record(const AZ::Data::AssetId& assetId, ReloadKind kind, AZStd::string_view uri);
        void MarkChanged(const AZ::Data::AssetId& assetId);
        AZStd::vector<ReloadChange> TakeChanges();

    private:
        AZStd::mutex m_mutex;
        AZStd::unordered_map<AZ::Data::AssetId, AZStd::vector<ReloadChange>> m_records;
        AZStd::vector<AZ::Data::AssetId> m_changed;
    };
}
