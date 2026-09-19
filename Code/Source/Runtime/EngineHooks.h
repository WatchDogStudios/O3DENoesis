#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/string/string.h>

namespace AzFramework
{
    class InputChannel;
}

//! The only runtime file allowed to call engine EBuses. Keeps the NightFox port (which deletes EBus) to a one-file patch.
namespace NoesisGUI::EngineHooks
{
    AZ::Data::AssetId FindAssetIdByPath(const AZStd::string& productPath);
    AZStd::string GetAssetPath(const AZ::Data::AssetId& assetId);
    void EnumerateProductPaths(const AZ::Data::AssetType& assetType, const AZStd::function<void(const AZStd::string&)>& callback);

    //! Keeps an engine subscription alive; destroying it disconnects.
    class Connection
    {
    public:
        virtual ~Connection() = default;
    };

    //! Handlers run on the main thread and return true to consume the event.
    AZStd::unique_ptr<Connection> ConnectInput(
        AZStd::function<bool(const AzFramework::InputChannel&)> onChannel, AZStd::function<bool(const AZStd::string&)> onText);

    //! Opens or closes the platform text-entry (on-screen keyboard where one exists).
    void SetTextEntryActive(bool active);

    //! Called when the asset catalog reports that a product changed on disk. May run off the main thread.
    AZStd::unique_ptr<Connection> ConnectAssetChanged(AZStd::function<void(const AZ::Data::AssetId&)> onChanged);
}
