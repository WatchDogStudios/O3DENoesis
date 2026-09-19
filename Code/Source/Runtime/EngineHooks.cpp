#include <Runtime/EngineHooks.h>

#include <AzCore/Asset/AssetManagerBus.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzFramework/Input/Buses/Requests/InputTextEntryRequestBus.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Input/Events/InputTextEventListener.h>

namespace NoesisGUI::EngineHooks
{
    AZ::Data::AssetId FindAssetIdByPath(const AZStd::string& productPath)
    {
        AZ::Data::AssetId assetId;
        AZ::Data::AssetCatalogRequestBus::BroadcastResult(
            assetId, &AZ::Data::AssetCatalogRequests::GetAssetIdByPath, productPath.c_str(), AZ::Data::AssetType{}, false);
        return assetId;
    }

    AZStd::string GetAssetPath(const AZ::Data::AssetId& assetId)
    {
        AZStd::string path;
        AZ::Data::AssetCatalogRequestBus::BroadcastResult(path, &AZ::Data::AssetCatalogRequests::GetAssetPathById, assetId);
        return path;
    }

    void EnumerateProductPaths(const AZ::Data::AssetType& assetType, const AZStd::function<void(const AZStd::string&)>& callback)
    {
        AZ::Data::AssetCatalogRequestBus::Broadcast(
            &AZ::Data::AssetCatalogRequests::EnumerateAssets,
            nullptr,
            [&](const AZ::Data::AssetId, const AZ::Data::AssetInfo& info)
            {
                if (info.m_assetType == assetType)
                {
                    callback(info.m_relativePath);
                }
            },
            nullptr);
    }

    namespace
    {
        class ChannelListener final : public AzFramework::InputChannelEventListener
        {
        public:
            explicit ChannelListener(AZStd::function<bool(const AzFramework::InputChannel&)> handler)
                : AzFramework::InputChannelEventListener(GetPriorityUI())
                , m_handler(AZStd::move(handler))
            {
                Connect();
            }

            ~ChannelListener() override
            {
                Disconnect();
            }

        private:
            bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override
            {
                return m_handler(inputChannel);
            }

            AZStd::function<bool(const AzFramework::InputChannel&)> m_handler;
        };

        class TextListener final : public AzFramework::InputTextEventListener
        {
        public:
            explicit TextListener(AZStd::function<bool(const AZStd::string&)> handler)
                : AzFramework::InputTextEventListener(GetPriorityUI())
                , m_handler(AZStd::move(handler))
            {
                Connect();
            }

            ~TextListener() override
            {
                Disconnect();
            }

        private:
            bool OnInputTextEventFiltered(const AZStd::string& textUTF8) override
            {
                return m_handler(textUTF8);
            }

            AZStd::function<bool(const AZStd::string&)> m_handler;
        };

        class InputConnection final : public Connection
        {
        public:
            InputConnection(AZStd::function<bool(const AzFramework::InputChannel&)> onChannel, AZStd::function<bool(const AZStd::string&)> onText)
                : m_channels(AZStd::move(onChannel))
                , m_text(AZStd::move(onText))
            {
            }

        private:
            ChannelListener m_channels;
            TextListener m_text;
        };

        class AssetChangedConnection final
            : public Connection
            , private AzFramework::AssetCatalogEventBus::Handler
        {
        public:
            explicit AssetChangedConnection(AZStd::function<void(const AZ::Data::AssetId&)> onChanged)
                : m_onChanged(AZStd::move(onChanged))
            {
                AzFramework::AssetCatalogEventBus::Handler::BusConnect();
            }

            ~AssetChangedConnection() override
            {
                AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
            }

        private:
            void OnCatalogAssetChanged(const AZ::Data::AssetId& assetId) override
            {
                m_onChanged(assetId);
            }

            AZStd::function<void(const AZ::Data::AssetId&)> m_onChanged;
        };
    }

    AZStd::unique_ptr<Connection> ConnectInput(
        AZStd::function<bool(const AzFramework::InputChannel&)> onChannel, AZStd::function<bool(const AZStd::string&)> onText)
    {
        return AZStd::make_unique<InputConnection>(AZStd::move(onChannel), AZStd::move(onText));
    }

    void SetTextEntryActive(bool active)
    {
        AzFramework::InputTextEntryRequestBus::Event(AzFramework::InputDeviceKeyboard::Id,
            [active](AzFramework::InputTextEntryRequests* requests)
            {
                if (active)
                {
                    requests->TextEntryStart(AzFramework::InputTextEntryRequests::VirtualKeyboardOptions());
                }
                else
                {
                    requests->TextEntryStop();
                }
            });
    }

    AZStd::unique_ptr<Connection> ConnectAssetChanged(AZStd::function<void(const AZ::Data::AssetId&)> onChanged)
    {
        return AZStd::make_unique<AssetChangedConnection>(AZStd::move(onChanged));
    }
}
