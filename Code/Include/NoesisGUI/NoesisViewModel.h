#pragma once

#include <AzCore/EBus/Event.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/std/string/string.h>

namespace AZ
{
    class ReflectContext;
}

namespace NoesisGUI
{
    class ViewModelObject;

    //! Script-built XAML DataContext. Setting a name that does not exist yet adds it; bindings update automatically.
    //! Copies share one underlying object. Every method locks the Noesis update mutex, so do not call these from
    //! inside a binding getter/setter or Can<Name> query (those run under that lock).
    class NoesisViewModel final
    {
    public:
        AZ_TYPE_INFO(NoesisGUI::NoesisViewModel, "{EB4A81B6-BB50-4981-A8E8-D3AC074838BF}");
        AZ_CLASS_ALLOCATOR(NoesisViewModel, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);

        NoesisViewModel();
        explicit NoesisViewModel(ViewModelObject* object);
        NoesisViewModel(const NoesisViewModel& other);
        NoesisViewModel& operator=(const NoesisViewModel& other);
        ~NoesisViewModel();

        void SetBool(const AZStd::string& name, bool value);
        void SetNumber(const AZStd::string& name, double value);
        void SetString(const AZStd::string& name, const AZStd::string& value);
        void SetColor(const AZStd::string& name, const AZ::Color& value);
        void SetImage(const AZStd::string& name, const AZStd::string& uri);
        bool GetBool(const AZStd::string& name) const;
        double GetNumber(const AZStd::string& name) const;
        AZStd::string GetString(const AZStd::string& name) const;
        AZ::Color GetColor(const AZStd::string& name) const;

        void SetChild(const AZStd::string& name, const NoesisViewModel& child);
        NoesisViewModel GetChild(const AZStd::string& name) const;

        void AddCollection(const AZStd::string& name);
        void AddToCollection(const AZStd::string& name, const NoesisViewModel& item);
        void RemoveFromCollection(const AZStd::string& name, AZ::u32 index);
        void ClearCollection(const AZStd::string& name);
        AZ::u32 GetCollectionCount(const AZStd::string& name) const;

        ViewModelObject* GetObject() const;

    private:
        ViewModelObject* m_object = nullptr;
    };
}
