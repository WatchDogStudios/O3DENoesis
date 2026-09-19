#pragma once

#include <AzCore/base.h>

namespace NoesisGUI::NoesisRuntime
{
    //! Refcounted Noesis::GUI::Init/Shutdown. The runtime system component and the asset builder each hold a reference,
    //! because AssetBuilder processes activate only builder components.
    void Acquire();
    void Release();
    AZ::u32 GetRefCount();
}
