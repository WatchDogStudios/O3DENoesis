#pragma once

#include <AzCore/std/optional.h>
#include <AzFramework/Input/Channels/InputChannelId.h>

#include <NsGui/InputEnums.h>

namespace NoesisGUI::KeyTable
{
    AZStd::optional<Noesis::Key> ToNoesisKey(const AzFramework::InputChannelId& channelId);
    AZStd::optional<Noesis::MouseButton> ToNoesisMouseButton(const AzFramework::InputChannelId& channelId);
}
