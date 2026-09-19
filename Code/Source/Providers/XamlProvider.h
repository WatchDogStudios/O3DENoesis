#pragma once

#include <NsGui/XamlProvider.h>

#include <Providers/ReloadRegistry.h>

namespace NoesisGUI
{
    class XamlProvider final : public Noesis::XamlProvider
    {
    public:
        explicit XamlProvider(ReloadRegistry* registry);

        Noesis::Ptr<Noesis::Stream> LoadXaml(const Noesis::Uri& uri) override;

    private:
        ReloadRegistry* m_registry;
    };
}
