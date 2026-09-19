#pragma once

#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/string/string.h>

#include <NsCore/BaseComponent.h>
#include <NsCore/Ptr.h>
#include <NsCore/TypeClassBuilder.h>
#include <NsGui/INotifyPropertyChanged.h>
#include <NsGui/ObservableCollection.h>

namespace NoesisGUI
{
    //! Noesis-side property bag behind NoesisViewModel. Its TypeClass is per instance and gains a property
    //! the first time a name is set, so bindings resolve names that script adds at any time.
    //! Called from Noesis bindings under the update mutex, so it never locks.
    class ViewModelObject final : public Noesis::BaseComponent, public Noesis::INotifyPropertyChanged
    {
    public:
        ViewModelObject();
        ~ViewModelObject() override;

        const Noesis::TypeClass* GetClassType() const override;
        Noesis::PropertyChangedEventHandler& PropertyChanged() override;

        Noesis::BaseComponent* GetValue(const AZStd::string& name) const;
        void SetValue(const AZStd::string& name, Noesis::BaseComponent* value);
        void AddCollection(const AZStd::string& name);
        Noesis::ObservableCollection<Noesis::BaseComponent>* GetCollection(const AZStd::string& name) const;

        NS_IMPLEMENT_INTERFACE_FIXUP

    private:
        Noesis::TypeClassBuilder m_type;
        AZStd::unordered_map<AZStd::string, Noesis::Ptr<Noesis::BaseComponent>> m_values;
        Noesis::PropertyChangedEventHandler m_propertyChanged;
    };
}
