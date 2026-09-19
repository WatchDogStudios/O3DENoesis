#include <Providers/TextureProvider.h>

#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/vector.h>
#include <Providers/AssetLoading.h>
#include <Providers/AssetUri.h>
#include <Providers/Premultiply.h>
#include <Render/AtomTexture.h>

#include <NsCore/String.h>
#include <NsGui/Uri.h>
#include <NsRender/RenderDevice.h>

namespace NoesisGUI
{
    namespace
    {
        // Named LoadStreamingImage (not LoadImage) to avoid colliding with the Windows LoadImage
        // macro/function when this unity TU pulls in windows.h via other RHI includes.
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> LoadStreamingImage(const Noesis::Uri& uri)
        {
            Noesis::FixedString<512> path;
            uri.GetPath(path);
            return LoadAssetBlocking<AZ::RPI::StreamingImageAsset>(AssetUri::ToProductPath(path.Str(), "streamingimage"));
        }
    }

    TextureProvider::TextureProvider(ReloadRegistry* registry)
        : m_registry(registry)
    {
    }

    Noesis::TextureInfo TextureProvider::GetTextureInfo(const Noesis::Uri& uri)
    {
        Noesis::TextureInfo info;
        if (AZ::Data::Asset<AZ::RPI::StreamingImageAsset> asset = LoadStreamingImage(uri))
        {
            const AZ::RHI::Size size = asset->GetImageDescriptor().m_size;
            info.width = size.m_width;
            info.height = size.m_height;
        }
        return info;
    }

    Noesis::Ptr<Noesis::Texture> TextureProvider::LoadTexture(const Noesis::Uri& uri, Noesis::RenderDevice* device)
    {
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> asset = LoadStreamingImage(uri);
        if (!asset)
        {
            return nullptr;
        }
        if (m_registry)
        {
            m_registry->Record(asset.GetId(), ReloadKind::Texture, uri.Str());
        }

        const AZ::RHI::ImageDescriptor& descriptor = asset->GetImageDescriptor();
        const AZ::u32 width = descriptor.m_size.m_width;
        const AZ::u32 height = descriptor.m_size.m_height;

        if (device && IsPremultipliableFormat(descriptor.m_format))
        {
            AZStd::vector<AZStd::vector<AZ::u8>> mips(descriptor.m_mipLevels);
            AZStd::vector<const void*> data(descriptor.m_mipLevels);
            bool complete = true;
            for (AZ::u16 mip = 0; mip < descriptor.m_mipLevels && complete; ++mip)
            {
                const AZStd::span<const uint8_t> source = asset->GetSubImageData(mip, 0);
                const size_t expected = size_t(AZStd::max(width >> mip, 1u)) * AZStd::max(height >> mip, 1u) * 4;
                complete = source.size() >= expected;
                mips[mip].assign(source.begin(), source.begin() + AZStd::min(source.size(), expected));
                PremultiplyRgba8(mips[mip]);
                data[mip] = mips[mip].data();
            }
            if (complete)
            {
                // The bytes stay sRGB-encoded and upload as UNORM: screen UI composites in gamma space (DeviceCaps::linearRendering is false).
                return device->CreateTexture(uri.Str(), width, height, descriptor.m_mipLevels, Noesis::TextureFormat::RGBA8, data.data());
            }
        }

        // ponytail: compressed/other formats still upload straight-alpha; add a premultiplying compressor path
        // (e.g. decompress-premultiply-recompress or a premultiplying image preset) if this warning fires in practice.
        AZ_WarningOnce("NoesisGUI", false,
            "Texture '%s' is not uncompressed RGBA8 and renders with straight alpha; use the UserInterface_Lossless image preset for UI images.", uri.Str());
        AZ::Data::Instance<AZ::RPI::StreamingImage> image = AZ::RPI::StreamingImage::FindOrCreate(asset);
        if (!image)
        {
            return nullptr;
        }
        return *new AtomTexture(image, width, height, descriptor.m_mipLevels, true);
    }
}
