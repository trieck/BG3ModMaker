#include "stdafx.h"

#include "DefaultRTFFormatter.h"
#include "JSONRTFFormatter.h"
#include "RTFFormatterRegistry.h"
#include "TextRTFFormatter.h"
#include "XmlRTFFormatter.h"

RTFFormatterRegistry::RTFFormatterRegistry()
{
    m_formatters[".xml"] = std::make_unique<XmlRTFFormatter>();
    m_formatters[".lsx"] = m_formatters[".xml"];
    m_formatters[".json"] = std::make_unique<JSONRTFFormatter>();
    m_formatters[".lsj"] = m_formatters[".json"];
    m_formatters[".txt"] = std::make_unique<TextRTFFormatter>();
    m_defaultFormatter = std::make_unique<DefaultRTFFormatter>();
}

RTFFormatterRegistry& RTFFormatterRegistry::GetInstance()
{
    static RTFFormatterRegistry instance;
    return instance;
}

RTFStreamFormatter::Ptr RTFFormatterRegistry::GetFormatter(const CString& path) const
{
    auto extension = ATLPath::FindExtension(path);

    CW2AEX<MAX_PATH> ansiExtension(extension);

    auto it = m_formatters.find(static_cast<LPSTR>(ansiExtension));
    if (it != m_formatters.end()) {
        return it->second;
    }

    return m_defaultFormatter;
}

RTFStreamFormatter::Ptr RTFFormatterRegistry::GetDefaultFormatter() const
{
    return m_defaultFormatter;
}

