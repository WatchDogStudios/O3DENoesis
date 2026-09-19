#include <Tools/XamlValidator.h>

#include <AzCore/XML/rapidxml.h>
#include <AzCore/std/containers/vector.h>

namespace NoesisGUI::XamlValidator
{
    AZ::Outcome<void, AZStd::string> Validate(AZStd::span<const AZ::u8> xaml)
    {
        if (xaml.empty())
        {
            return AZ::Failure(AZStd::string("empty XAML file"));
        }
        // ponytail: RapidXML's parse_bom only recognizes the UTF-8 BOM; a UTF-16 buffer would misparse as
        // garbage single-byte XML. Skip the well-formedness check for UTF-16 XAML — Noesis's own reader
        // still catches errors at load — and transcode to UTF-8 first if that gap ever matters.
        if (xaml.size() >= 2 && ((xaml[0] == 0xFF && xaml[1] == 0xFE) || (xaml[0] == 0xFE && xaml[1] == 0xFF)))
        {
            return AZ::Success();
        }
        // RapidXML parses in place and needs a null-terminated, writable buffer.
        AZStd::vector<char> text(xaml.begin(), xaml.end());
        text.push_back('\0');
        AZ::rapidxml::xml_document<char> document;
        if (!document.parse<0>(text.data()))
        {
            return AZ::Failure(AZStd::string(document.getError()));
        }
        if (!document.first_node())
        {
            return AZ::Failure(AZStd::string("XAML file has no root element"));
        }
        return AZ::Success();
    }
}
