#pragma once

#include <Atom/RPI.Public/Image/AttachmentImage.h>
#include <AzCore/Asset/AssetCommon.h>

#include <NsCore/Ptr.h>
#include <NsRender/RenderTarget.h>
#include <NsRender/Texture.h>

namespace NoesisGUI
{
    class AtomTexture final : public Noesis::Texture
    {
    public:
        AtomTexture(AZ::Data::Instance<AZ::RPI::Image> image, AZ::u32 width, AZ::u32 height, AZ::u32 levels, bool hasAlpha);

        uint32_t GetWidth() const override { return m_width; }
        uint32_t GetHeight() const override { return m_height; }
        bool HasMipMaps() const override { return m_levels > 1; }
        bool IsInverted() const override { return false; }
        bool HasAlpha() const override { return m_hasAlpha; }

        const AZ::Data::Instance<AZ::RPI::Image>& GetImage() const { return m_image; }
        AZ::RPI::AttachmentImage* GetAttachmentImage() const;
        AZ::u64 GetId() const { return m_id; }
        bool IsRenderTarget() const { return m_isRenderTarget; }
        void MarkRenderTarget() { m_isRenderTarget = true; }

    private:
        AZ::Data::Instance<AZ::RPI::Image> m_image;
        AZ::u32 m_width;
        AZ::u32 m_height;
        AZ::u32 m_levels;
        bool m_hasAlpha;
        bool m_isRenderTarget = false;
        AZ::u64 m_id;
    };

    class AtomRenderTarget final : public Noesis::RenderTarget
    {
    public:
        AtomRenderTarget(Noesis::Ptr<AtomTexture> color, AZ::Data::Instance<AZ::RPI::AttachmentImage> stencil);

        Noesis::Texture* GetTexture() override { return m_color.GetPtr(); }
        AtomTexture& GetColor() const { return *m_color; }
        AZ::RPI::AttachmentImage* GetStencil() const { return m_stencil.get(); }
        const AZ::Data::Instance<AZ::RPI::AttachmentImage>& GetStencilInstance() const { return m_stencil; }

    private:
        Noesis::Ptr<AtomTexture> m_color;
        AZ::Data::Instance<AZ::RPI::AttachmentImage> m_stencil;
    };
}
