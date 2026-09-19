#pragma once

namespace NoesisGUI
{
    inline constexpr const char* NoesisGUISystemComponentTypeId = "{8E1A5A7B-3F7C-4C35-9C9E-0E0D5C6E3B21}";
    inline constexpr const char* NoesisGUIEditorSystemComponentTypeId = "{0F4F6B8D-6A3E-4F59-8D5D-7A6C1B2E4F32}";
    inline constexpr const char* NoesisGUIModuleInterfaceTypeId = "{C3B7E2A1-9D4F-4E6B-A8C5-2F1D0E9B7A43}";
    inline constexpr const char* NoesisGUIModuleTypeId = "{A1D9F3C2-5B8E-4A6D-9C7F-3E2B1A0D8F54}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* NoesisGUIEditorModuleTypeId = NoesisGUIModuleTypeId;
}
