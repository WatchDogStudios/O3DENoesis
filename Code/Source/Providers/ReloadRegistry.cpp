#include <Providers/ReloadRegistry.h>

#include <AzCore/std/algorithm.h>

namespace NoesisGUI
{
    void ReloadRegistry::Record(const AZ::Data::AssetId& assetId, ReloadKind kind, AZStd::string_view uri)
    {
        AZStd::scoped_lock lock(m_mutex);
        AZStd::vector<ReloadChange>& records = m_records[assetId];
        const bool known = AZStd::any_of(records.begin(), records.end(),
            [kind, uri](const ReloadChange& record) { return record.m_kind == kind && record.m_uri == uri; });
        if (!known)
        {
            records.push_back({ kind, AZStd::string(uri) });
        }
    }

    void ReloadRegistry::MarkChanged(const AZ::Data::AssetId& assetId)
    {
        AZStd::scoped_lock lock(m_mutex);
        if (AZStd::find(m_changed.begin(), m_changed.end(), assetId) == m_changed.end())
        {
            m_changed.push_back(assetId);
        }
    }

    AZStd::vector<ReloadChange> ReloadRegistry::TakeChanges()
    {
        AZStd::scoped_lock lock(m_mutex);
        AZStd::vector<ReloadChange> changes;
        for (const AZ::Data::AssetId& assetId : m_changed)
        {
            if (auto it = m_records.find(assetId); it != m_records.end())
            {
                changes.insert(changes.end(), it->second.begin(), it->second.end());
            }
        }
        m_changed.clear();
        return changes;
    }
}
