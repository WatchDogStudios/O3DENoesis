#include <NoesisGUIModuleInterface.h>

#include <AzCore/Memory/Memory.h>
#include <Components/NoesisViewComponent.h>
#include <NoesisGUI/NoesisGUITypeIds.h>
#include <Runtime/NoesisGUISystemComponent.h>

namespace NoesisGUI
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(NoesisGUIModuleInterface, "NoesisGUIModuleInterface", NoesisGUIModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(NoesisGUIModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(NoesisGUIModuleInterface, AZ::SystemAllocator);

    NoesisGUIModuleInterface::NoesisGUIModuleInterface()
    {
        m_descriptors.insert(m_descriptors.end(), {
            NoesisGUISystemComponent::CreateDescriptor(),
            NoesisViewComponent::CreateDescriptor(),
        });
    }

    AZ::ComponentTypeList NoesisGUIModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{ azrtti_typeid<NoesisGUISystemComponent>() };
    }
}
