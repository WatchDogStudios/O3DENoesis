#include <NoesisGUI/NoesisXamlAsset.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace NoesisGUI
{
    void NoesisXamlAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NoesisXamlAsset>()->Version(1)->Field("Data", &NoesisXamlAsset::m_data);
        }
    }
}
