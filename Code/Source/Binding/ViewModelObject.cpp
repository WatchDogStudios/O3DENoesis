#include <Binding/ViewModelObject.h>

#include <NsCore/Symbol.h>
#include <NsCore/TypeOf.h>
#include <NsCore/TypeProperty.h>

namespace NoesisGUI
{
    namespace
    {
        class ViewModelProperty final : public Noesis::TypeProperty
        {
        public:
            explicit ViewModelProperty(const AZStd::string& name)
                : Noesis::TypeProperty(Noesis::Symbol(name.c_str()), Noesis::TypeOf<Noesis::BaseComponent>())
                , m_name(name)
            {
            }

            void* GetContent(const void*) const override
            {
                return nullptr;
            }

            Noesis::Ptr<Noesis::BaseComponent> GetComponent(const void* ptr) const override
            {
                return Noesis::Ptr<Noesis::BaseComponent>(Object(ptr)->GetValue(m_name));
            }

            void SetComponent(void* ptr, Noesis::BaseComponent* value) const override
            {
                const_cast<ViewModelObject*>(Object(ptr))->SetValue(m_name, value);
            }

        private:
            static const ViewModelObject* Object(const void* ptr)
            {
                return static_cast<const ViewModelObject*>(static_cast<const Noesis::BaseComponent*>(ptr));
            }

            AZStd::string m_name;
        };
    }

    ViewModelObject::ViewModelObject()
        : m_type(Noesis::Symbol("NoesisViewModel"), false)
    {
        m_type.AddBase(Noesis::TypeOf<Noesis::BaseComponent>());
        // Noesis finds INotifyPropertyChanged through the TypeClass, not C++ inheritance, for dynamic types.
        // Same pointer-adjustment trick Noesis::TypeClassCreator::CalculateParentOffset uses for static types.
        ViewModelObject* const probe = reinterpret_cast<ViewModelObject*>(0x10000000);
        const auto offset = static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(static_cast<Noesis::INotifyPropertyChanged*>(probe)) -
            reinterpret_cast<uintptr_t>(probe));
        m_type.AddInterface(Noesis::TypeOf<Noesis::INotifyPropertyChanged>(), offset);
    }

    ViewModelObject::~ViewModelObject() = default;

    const Noesis::TypeClass* ViewModelObject::GetClassType() const
    {
        return &m_type;
    }

    Noesis::PropertyChangedEventHandler& ViewModelObject::PropertyChanged()
    {
        return m_propertyChanged;
    }

    Noesis::BaseComponent* ViewModelObject::GetValue(const AZStd::string& name) const
    {
        auto it = m_values.find(name);
        return it != m_values.end() ? it->second.GetPtr() : nullptr;
    }

    void ViewModelObject::SetValue(const AZStd::string& name, Noesis::BaseComponent* value)
    {
        auto [it, added] = m_values.try_emplace(name);
        if (added)
        {
            m_type.AddProperty(new ViewModelProperty(name));
        }
        it->second.Reset(value);
        if (m_propertyChanged)
        {
            if (added)
            {
                // A binding that resolved before this name existed marked its path permanently broken and
                // never rechecks the TypeClass, even for the exact property name. Noesis honors an empty
                // property name as "the whole object may have changed" (the WPF convention), which forces
                // those broken paths to re-resolve now that the property is there. Fire it first so the
                // exact-name event below is the last (and therefore observable) notification for this call.
                m_propertyChanged(this, Noesis::PropertyChangedEventArgs(Noesis::Symbol::Null()));
            }
            m_propertyChanged(this, Noesis::PropertyChangedEventArgs(Noesis::Symbol(name.c_str())));
        }
    }

    void ViewModelObject::AddCollection(const AZStd::string& name)
    {
        if (!GetCollection(name))
        {
            SetValue(name, Noesis::MakePtr<Noesis::ObservableCollection<Noesis::BaseComponent>>().GetPtr());
        }
    }

    Noesis::ObservableCollection<Noesis::BaseComponent>* ViewModelObject::GetCollection(const AZStd::string& name) const
    {
        return Noesis::DynamicCast<Noesis::ObservableCollection<Noesis::BaseComponent>*>(GetValue(name));
    }
}
