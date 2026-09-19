#include <NoesisGUI/NoesisViewModel.h>

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/parallel/mutex.h>
#include <Binding/ValueConversion.h>
#include <Binding/ViewModelObject.h>
#include <Render/NoesisFeatureProcessor.h>

namespace NoesisGUI
{
    namespace
    {
        template<class T>
        T Get(const ViewModelObject& object, const AZStd::string& name, T fallback = T{})
        {
            ValueConversion::FromNoesis(object.GetValue(name), azrtti_typeid<T>(), &fallback);
            return fallback;
        }

        template<class T>
        void Set(ViewModelObject& object, const AZStd::string& name, const T& value)
        {
            object.SetValue(name, ValueConversion::ToNoesis(azrtti_typeid<T>(), &value).GetPtr());
        }

        // ViewModelObject's TypeClass is built per instance (Noesis::TypeClassBuilder), so it has no
        // StaticGetClassType and Noesis::DynamicCast<ViewModelObject*> would not compile. Identify it by
        // the class name every ViewModelObject registers instead.
        ViewModelObject* AsViewModelObject(Noesis::BaseComponent* value)
        {
            if (value && AZStd::string_view(value->GetClassType()->GetName()) == "NoesisViewModel")
            {
                return static_cast<ViewModelObject*>(value);
            }
            return nullptr;
        }
    }

    NoesisViewModel::NoesisViewModel()
        : m_object(new ViewModelObject())
    {
    }

    NoesisViewModel::NoesisViewModel(ViewModelObject* object)
        : m_object(object ? object : new ViewModelObject())
    {
        if (object)
        {
            m_object->AddReference();
        }
    }

    NoesisViewModel::NoesisViewModel(const NoesisViewModel& other)
        : m_object(other.m_object)
    {
        m_object->AddReference();
    }

    NoesisViewModel& NoesisViewModel::operator=(const NoesisViewModel& other)
    {
        other.m_object->AddReference();
        m_object->Release();
        m_object = other.m_object;
        return *this;
    }

    NoesisViewModel::~NoesisViewModel()
    {
        // Script can drop the last copy during any tick; Noesis state is only touched under the update mutex.
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        m_object->Release();
    }

    void NoesisViewModel::SetBool(const AZStd::string& name, bool value)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        Set(*m_object, name, value);
    }

    void NoesisViewModel::SetNumber(const AZStd::string& name, double value)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        Set(*m_object, name, value);
    }

    void NoesisViewModel::SetString(const AZStd::string& name, const AZStd::string& value)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        Set(*m_object, name, value);
    }

    void NoesisViewModel::SetColor(const AZStd::string& name, const AZ::Color& value)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        Set(*m_object, name, value);
    }

    void NoesisViewModel::SetImage(const AZStd::string& name, const AZStd::string& uri)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        m_object->SetValue(name, ValueConversion::MakeImage(uri).GetPtr());
    }

    bool NoesisViewModel::GetBool(const AZStd::string& name) const
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        return Get<bool>(*m_object, name);
    }

    double NoesisViewModel::GetNumber(const AZStd::string& name) const
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        return Get<double>(*m_object, name);
    }

    AZStd::string NoesisViewModel::GetString(const AZStd::string& name) const
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        return Get<AZStd::string>(*m_object, name);
    }

    AZ::Color NoesisViewModel::GetColor(const AZStd::string& name) const
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        return Get<AZ::Color>(*m_object, name, AZ::Color::CreateZero());
    }

    void NoesisViewModel::SetChild(const AZStd::string& name, const NoesisViewModel& child)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        m_object->SetValue(name, child.m_object);
    }

    NoesisViewModel NoesisViewModel::GetChild(const AZStd::string& name) const
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        return NoesisViewModel(AsViewModelObject(m_object->GetValue(name)));
    }

    void NoesisViewModel::AddCollection(const AZStd::string& name)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        m_object->AddCollection(name);
    }

    void NoesisViewModel::AddToCollection(const AZStd::string& name, const NoesisViewModel& item)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        m_object->AddCollection(name);
        m_object->GetCollection(name)->Add(item.m_object);
    }

    void NoesisViewModel::RemoveFromCollection(const AZStd::string& name, AZ::u32 index)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        auto* collection = m_object->GetCollection(name);
        if (collection && index < static_cast<AZ::u32>(collection->Count()))
        {
            collection->RemoveAt(index);
        }
    }

    void NoesisViewModel::ClearCollection(const AZStd::string& name)
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        if (auto* collection = m_object->GetCollection(name))
        {
            collection->Clear();
        }
    }

    AZ::u32 NoesisViewModel::GetCollectionCount(const AZStd::string& name) const
    {
        AZStd::scoped_lock lock(NoesisFeatureProcessor::GetUpdateMutex());
        auto* collection = m_object->GetCollection(name);
        return collection ? static_cast<AZ::u32>(collection->Count()) : 0;
    }

    ViewModelObject* NoesisViewModel::GetObject() const
    {
        return m_object;
    }

    void NoesisViewModel::Reflect(AZ::ReflectContext*)
    {
    }
}
