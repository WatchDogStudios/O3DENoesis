#pragma once

#include <Runtime/NoesisGUISystemComponent.h>

namespace NoesisGUI
{
    class NoesisGUIEditorSystemComponent : public NoesisGUISystemComponent
    {
        using BaseSystemComponent = NoesisGUISystemComponent;

    public:
        AZ_COMPONENT_DECL(NoesisGUIEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
    };
}
