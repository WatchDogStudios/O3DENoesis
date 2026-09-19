#pragma once

#include <NsGui/CachedFontProvider.h>

namespace NoesisGUI
{
    class FontProvider final : public Noesis::CachedFontProvider
    {
    protected:
        void ScanFolder(const Noesis::Uri& folder) override;
        Noesis::Ptr<Noesis::Stream> OpenFont(const Noesis::Uri& folder, const char* filename) const override;
    };
}
