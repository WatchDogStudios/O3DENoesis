#include <AzCore/Math/Color.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/std/string/string.h>
#include <AzTest/AzTest.h>
#include <Binding/ViewModelObject.h>
#include <NoesisGUI/NoesisViewModel.h>

#include <NsGui/INotifyPropertyChanged.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/ItemsControl.h>
#include <NsGui/ItemCollection.h>
#include <NsGui/IView.h>
#include <NsGui/StackPanel.h>
#include <NsGui/TextBlock.h>
#include <NsGui/UIElementCollection.h>

namespace NoesisGUI::Test
{
    class NoesisViewModelTests : public UnitTest::LeakDetectionFixture
    {
    protected:
        static Noesis::Ptr<Noesis::TextBlock> ParseTextBlock(const char* binding)
        {
            AZStd::string xaml = AZStd::string::format(
                R"(<TextBlock xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" Text="%s"/>)", binding);
            return Noesis::GUI::ParseXaml<Noesis::TextBlock>(xaml.c_str());
        }

        // A bare ParseXaml element does not evaluate bindings; only content laid out inside a view does.
        static Noesis::Ptr<Noesis::IView> MakeView(Noesis::FrameworkElement* content)
        {
            Noesis::Ptr<Noesis::IView> view = Noesis::GUI::CreateView(content);
            view->SetSize(100, 100);
            view->Update(0.0);
            return view;
        }
    };

    TEST_F(NoesisViewModelTests, ValuesRoundTrip)
    {
        NoesisViewModel vm;
        vm.SetBool("Flag", true);
        vm.SetNumber("Count", 3.5);
        vm.SetString("Title", "Hello");
        vm.SetColor("Tint", AZ::Color(1.0f, 0.5f, 0.25f, 1.0f));
        EXPECT_TRUE(vm.GetBool("Flag"));
        EXPECT_DOUBLE_EQ(vm.GetNumber("Count"), 3.5);
        EXPECT_EQ(vm.GetString("Title"), "Hello");
        EXPECT_TRUE(vm.GetColor("Tint").IsClose(AZ::Color(1.0f, 0.5f, 0.25f, 1.0f)));
        EXPECT_FALSE(vm.GetBool("Missing"));
        EXPECT_EQ(vm.GetString("Missing"), "");
    }

    TEST_F(NoesisViewModelTests, CopiesShareTheObject)
    {
        NoesisViewModel a;
        NoesisViewModel b = a;
        b.SetString("Title", "Shared");
        EXPECT_EQ(a.GetString("Title"), "Shared");
        EXPECT_EQ(a.GetObject(), b.GetObject());
    }

    TEST_F(NoesisViewModelTests, SetRaisesPropertyChanged)
    {
        NoesisViewModel vm;
        AZStd::string changed;
        vm.GetObject()->PropertyChanged() += [&changed](Noesis::BaseComponent*, const Noesis::PropertyChangedEventArgs& args)
        {
            changed = args.propertyName.Str();
        };
        vm.SetNumber("Count", 1.0);
        EXPECT_EQ(changed, "Count");
    }

    TEST_F(NoesisViewModelTests, XamlBindsToValuesAddedBeforeAndAfterDataContext)
    {
        NoesisViewModel vm;
        vm.SetString("Title", "Before");
        Noesis::Ptr<Noesis::TextBlock> title = ParseTextBlock("{Binding Title}");
        Noesis::Ptr<Noesis::TextBlock> late = ParseTextBlock("{Binding Late}");
        title->SetDataContext(vm.GetObject());
        late->SetDataContext(vm.GetObject());
        Noesis::Ptr<Noesis::IView> titleView = MakeView(title);
        Noesis::Ptr<Noesis::IView> lateView = MakeView(late);
        EXPECT_STREQ(title->GetText(), "Before");

        vm.SetString("Title", "After");
        vm.SetString("Late", "Arrived");
        late->SetDataContext(nullptr);
        late->SetDataContext(vm.GetObject());
        titleView->Update(0.0);
        lateView->Update(0.0);
        EXPECT_STREQ(title->GetText(), "After");
        EXPECT_STREQ(late->GetText(), "Arrived");
    }

    TEST_F(NoesisViewModelTests, ChildPathBinds)
    {
        NoesisViewModel vm;
        NoesisViewModel child;
        child.SetString("Name", "Nested");
        vm.SetChild("Child", child);
        EXPECT_EQ(vm.GetChild("Child").GetObject(), child.GetObject());

        Noesis::Ptr<Noesis::TextBlock> text = ParseTextBlock("{Binding Child.Name}");
        text->SetDataContext(vm.GetObject());
        Noesis::Ptr<Noesis::IView> view = MakeView(text);
        EXPECT_STREQ(text->GetText(), "Nested");
    }

    TEST_F(NoesisViewModelTests, CollectionsDriveItemsSource)
    {
        NoesisViewModel vm;
        vm.AddCollection("Items");
        Noesis::Ptr<Noesis::ItemsControl> list = Noesis::GUI::ParseXaml<Noesis::ItemsControl>(
            R"(<ItemsControl xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" ItemsSource="{Binding Items}"/>)");
        list->SetDataContext(vm.GetObject());
        Noesis::Ptr<Noesis::IView> view = MakeView(list);

        NoesisViewModel first;
        NoesisViewModel second;
        vm.AddToCollection("Items", first);
        vm.AddToCollection("Items", second);
        view->Update(0.0);
        EXPECT_EQ(vm.GetCollectionCount("Items"), 2u);
        EXPECT_EQ(list->GetItems()->Count(), 2);

        vm.RemoveFromCollection("Items", 0);
        view->Update(0.0);
        EXPECT_EQ(list->GetItems()->Count(), 1);
        vm.ClearCollection("Items");
        view->Update(0.0);
        EXPECT_EQ(vm.GetCollectionCount("Items"), 0u);
        EXPECT_EQ(list->GetItems()->Count(), 0);
    }
}
