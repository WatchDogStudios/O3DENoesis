#include <NoesisGUIModuleInterface.h>

#include <NoesisGUI/NoesisGUITypeIds.h>

namespace NoesisGUI
{
    class NoesisGUIModule : public NoesisGUIModuleInterface
    {
    public:
        AZ_RTTI(NoesisGUIModule, NoesisGUIModuleTypeId, NoesisGUIModuleInterface);
        AZ_CLASS_ALLOCATOR(NoesisGUIModule, AZ::SystemAllocator);
    };
}

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), NoesisGUI::NoesisGUIModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_NoesisGUI, NoesisGUI::NoesisGUIModule)
#endif
