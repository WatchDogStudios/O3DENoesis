#pragma once

#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/std/string/string.h>

#include <NsCore/Ptr.h>

namespace Noesis
{
    class BaseComponent;
}

namespace NoesisGUI::ValueConversion
{
    bool IsSupported(const AZ::TypeId& typeId);
    Noesis::Ptr<Noesis::BaseComponent> ToNoesis(const AZ::TypeId& typeId, const void* value);
    bool FromNoesis(Noesis::BaseComponent* boxed, const AZ::TypeId& typeId, void* value);
    Noesis::Ptr<Noesis::BaseComponent> MakeImage(const AZStd::string& uri);
}
