#include "stdafx.h"

#include "DefaultRTFFormatter.h"
#include "RTFFormatterRegistry.h"
#include "XmlRTFFormatter.h"

RTFFormatterRegistry::RTFFormatterRegistry()
{
    m_formatters[".xml"] = std::make_unique<XmlRTFFormatter>();
    m_formatters[".lsx"] = m_formatters[".xml"];
    m_formatters[".xsl"] = m_formatters[".xml"];
    m_formatters[".xslt"] = m_formatters[".xml"];
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

    return m_defaultFormatter ? m_defaultFormatter : std::make_shared<DefaultRTFFormatter>();
}

