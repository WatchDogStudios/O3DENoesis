#include <Binding/ValueConversion.h>

#include <AzCore/Math/Color.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/RTTI/RTTI.h>

#include <NsCore/BaseComponent.h>
#include <NsCore/Boxing.h>
#include <NsCore/String.h>
#include <NsDrawing/Color.h>
#include <NsDrawing/Point.h>
#include <NsDrawing/Thickness.h>
#include <NsGui/BitmapImage.h>
#include <NsGui/Uri.h>

namespace NoesisGUI::ValueConversion
{
    namespace
    {
        template<class Az, class Ns>
        bool BoxNumeric(const AZ::TypeId& typeId, const void* in, Noesis::Ptr<Noesis::BaseComponent>& out)
        {
            if (typeId != azrtti_typeid<Az>())
            {
                return false;
            }
            out = Noesis::Boxing::Box<Ns>(static_cast<Ns>(*static_cast<const Az*>(in)));
            return true;
        }

        template<class Az, class Ns>
        bool UnboxNumeric(const AZ::TypeId& typeId, Noesis::BaseComponent* boxed, void* out, bool& ok)
        {
            if (typeId != azrtti_typeid<Az>())
            {
                return false;
            }
            ok = Noesis::Boxing::CanUnbox<Ns>(boxed);
            if (ok)
            {
                *static_cast<Az*>(out) = static_cast<Az>(Noesis::Boxing::Unbox<Ns>(boxed));
            }
            return true;
        }
    }

    bool IsSupported(const AZ::TypeId& typeId)
    {
        return typeId == azrtti_typeid<bool>() || typeId == azrtti_typeid<AZ::s8>() || typeId == azrtti_typeid<AZ::s16>() ||
            typeId == azrtti_typeid<AZ::s32>() || typeId == azrtti_typeid<AZ::s64>() || typeId == azrtti_typeid<AZ::u8>() ||
            typeId == azrtti_typeid<AZ::u16>() || typeId == azrtti_typeid<AZ::u32>() || typeId == azrtti_typeid<AZ::u64>() ||
            typeId == azrtti_typeid<float>() || typeId == azrtti_typeid<double>() || typeId == azrtti_typeid<AZStd::string>() ||
            typeId == azrtti_typeid<AZ::Color>() || typeId == azrtti_typeid<AZ::Vector2>() || typeId == azrtti_typeid<AZ::Vector4>();
    }

    Noesis::Ptr<Noesis::BaseComponent> ToNoesis(const AZ::TypeId& typeId, const void* value)
    {
        Noesis::Ptr<Noesis::BaseComponent> out;
        if (BoxNumeric<bool, bool>(typeId, value, out) || BoxNumeric<AZ::s8, int8_t>(typeId, value, out) ||
            BoxNumeric<AZ::s16, int16_t>(typeId, value, out) || BoxNumeric<AZ::s32, int32_t>(typeId, value, out) ||
            BoxNumeric<AZ::s64, int64_t>(typeId, value, out) || BoxNumeric<AZ::u8, uint8_t>(typeId, value, out) ||
            BoxNumeric<AZ::u16, uint16_t>(typeId, value, out) || BoxNumeric<AZ::u32, uint32_t>(typeId, value, out) ||
            BoxNumeric<AZ::u64, uint64_t>(typeId, value, out) || BoxNumeric<float, float>(typeId, value, out) ||
            BoxNumeric<double, double>(typeId, value, out))
        {
            return out;
        }
        if (typeId == azrtti_typeid<AZStd::string>())
        {
            return Noesis::Boxing::Box(static_cast<const AZStd::string*>(value)->c_str());
        }
        if (typeId == azrtti_typeid<AZ::Color>())
        {
            const auto& c = *static_cast<const AZ::Color*>(value);
            return Noesis::Boxing::Box(Noesis::Color(c.GetR(), c.GetG(), c.GetB(), c.GetA()));
        }
        if (typeId == azrtti_typeid<AZ::Vector2>())
        {
            const auto& v = *static_cast<const AZ::Vector2*>(value);
            return Noesis::Boxing::Box(Noesis::Point(v.GetX(), v.GetY()));
        }
        if (typeId == azrtti_typeid<AZ::Vector4>())
        {
            const auto& v = *static_cast<const AZ::Vector4*>(value);
            return Noesis::Boxing::Box(Noesis::Thickness(v.GetX(), v.GetY(), v.GetZ(), v.GetW()));
        }
        return nullptr;
    }

    bool FromNoesis(Noesis::BaseComponent* boxed, const AZ::TypeId& typeId, void* value)
    {
        if (!boxed)
        {
            return false;
        }
        bool ok = false;
        if (UnboxNumeric<bool, bool>(typeId, boxed, value, ok) || UnboxNumeric<AZ::s8, int8_t>(typeId, boxed, value, ok) ||
            UnboxNumeric<AZ::s16, int16_t>(typeId, boxed, value, ok) || UnboxNumeric<AZ::s32, int32_t>(typeId, boxed, value, ok) ||
            UnboxNumeric<AZ::s64, int64_t>(typeId, boxed, value, ok) || UnboxNumeric<AZ::u8, uint8_t>(typeId, boxed, value, ok) ||
            UnboxNumeric<AZ::u16, uint16_t>(typeId, boxed, value, ok) || UnboxNumeric<AZ::u32, uint32_t>(typeId, boxed, value, ok) ||
            UnboxNumeric<AZ::u64, uint64_t>(typeId, boxed, value, ok) || UnboxNumeric<float, float>(typeId, boxed, value, ok) ||
            UnboxNumeric<double, double>(typeId, boxed, value, ok))
        {
            return ok;
        }
        if (typeId == azrtti_typeid<AZStd::string>() && Noesis::Boxing::CanUnbox<Noesis::String>(boxed))
        {
            *static_cast<AZStd::string*>(value) = Noesis::Boxing::Unbox<Noesis::String>(boxed).Str();
            return true;
        }
        if (typeId == azrtti_typeid<AZ::Color>() && Noesis::Boxing::CanUnbox<Noesis::Color>(boxed))
        {
            const Noesis::Color c = Noesis::Boxing::Unbox<Noesis::Color>(boxed);
            *static_cast<AZ::Color*>(value) = AZ::Color(c.r, c.g, c.b, c.a);
            return true;
        }
        if (typeId == azrtti_typeid<AZ::Vector2>() && Noesis::Boxing::CanUnbox<Noesis::Point>(boxed))
        {
            const Noesis::Point p = Noesis::Boxing::Unbox<Noesis::Point>(boxed);
            *static_cast<AZ::Vector2*>(value) = AZ::Vector2(p.x, p.y);
            return true;
        }
        if (typeId == azrtti_typeid<AZ::Vector4>() && Noesis::Boxing::CanUnbox<Noesis::Thickness>(boxed))
        {
            const Noesis::Thickness t = Noesis::Boxing::Unbox<Noesis::Thickness>(boxed);
            *static_cast<AZ::Vector4*>(value) = AZ::Vector4(t.left, t.top, t.right, t.bottom);
            return true;
        }
        return false;
    }

    Noesis::Ptr<Noesis::BaseComponent> MakeImage(const AZStd::string& uri)
    {
        if (uri.empty())
        {
            return nullptr;
        }
        return *new Noesis::BitmapImage(Noesis::Uri(uri.c_str()));
    }
}
