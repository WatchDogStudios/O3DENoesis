#include <NoesisGUI/NoesisFontAsset.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace NoesisGUI
{
    void NoesisFontAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisFontAsset>()->Version(1)->Field("Data", &NoesisFontAsset::m_data);
        }
    }
}
