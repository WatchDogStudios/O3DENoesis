#include <NoesisGUIModuleInterface.h>

#include <NoesisGUI/NoesisGUITypeIds.h>
#include <Tools/NoesisGUIBuilderComponent.h>
#include <Tools/NoesisGUIEditorSystemComponent.h>

namespace NoesisGUI
{
    class NoesisGUIEditorModule : public NoesisGUIModuleInterface
    {
    public:
        AZ_RTTI(NoesisGUIEditorModule, NoesisGUIEditorModuleTypeId, NoesisGUIModuleInterface);
        AZ_CLASS_ALLOCATOR(NoesisGUIEditorModule, AZ::SystemAllocator);

        NoesisGUIEditorModule()
        {
            m_descriptors.insert(m_descriptors.end(), {
                NoesisGUIEditorSystemComponent::CreateDescriptor(),
                NoesisGUIBuilderComponent::CreateDescriptor(),
            });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{ azrtti_typeid<NoesisGUIEditorSystemComponent>() };
        }
    };
}

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), NoesisGUI::NoesisGUIEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_NoesisGUI_Editor, NoesisGUI::NoesisGUIEditorModule)
#endif
