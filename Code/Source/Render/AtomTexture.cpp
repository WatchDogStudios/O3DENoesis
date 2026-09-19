#include <Render/AtomTexture.h>

#include <AzCore/std/parallel/atomic.h>

namespace NoesisGUI
{
    namespace
    {
        AZStd::atomic<AZ::u64> s_nextTextureId{ 1 };
    }

    AtomTexture::AtomTexture(AZ::Data::Instance<AZ::RPI::Image> image, AZ::u32 width, AZ::u32 height, AZ::u32 levels, bool hasAlpha)
        : m_image(AZStd::move(image))
        , m_width(width)
        , m_height(height)
        , m_levels(levels)
        , m_hasAlpha(hasAlpha)
        , m_id(s_nextTextureId++)
    {
    }

    AZ::RPI::AttachmentImage* AtomTexture::GetAttachmentImage() const
    {
        return azrtti_cast<AZ::RPI::AttachmentImage*>(m_image.get());
    }

    AtomRenderTarget::AtomRenderTarget(Noesis::Ptr<AtomTexture> color, AZ::Data::Instance<AZ::RPI::AttachmentImage> stencil)
        : m_color(AZStd::move(color))
        , m_stencil(AZStd::move(stencil))
    {
    }
}
