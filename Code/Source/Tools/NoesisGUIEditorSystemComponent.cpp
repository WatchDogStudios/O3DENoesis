#include <Tools/NoesisGUIEditorSystemComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <NoesisGUI/NoesisGUITypeIds.h>

namespace NoesisGUI
{
    AZ_COMPONENT_IMPL(
        NoesisGUIEditorSystemComponent, "NoesisGUIEditorSystemComponent", NoesisGUIEditorSystemComponentTypeId, BaseSystemComponent);

    void NoesisGUIEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisGUIEditorSystemComponent, NoesisGUISystemComponent>()->Version(0);
        }
    }

    void NoesisGUIEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("NoesisGUIEditorService"));
    }

    void NoesisGUIEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("NoesisGUIEditorService"));
    }

    void NoesisGUIEditorSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }
}
