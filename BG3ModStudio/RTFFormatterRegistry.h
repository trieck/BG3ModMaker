#pragma once

#include "RTFStreamFormatter.h"

class RTFFormatterRegistry
{
public:
    RTFFormatterRegistry();
    ~RTFFormatterRegistry() = default;

    static RTFFormatterRegistry& GetInstance();

    RTFStreamFormatter::Ptr GetFormatter(const CString& path) const;
    RTFStreamFormatter::Ptr GetDefaultFormatter() const;

private:
    std::unordered_map<std::string, RTFStreamFormatter::Ptr> m_formatters;
    RTFStreamFormatter::Ptr m_defaultFormatter;
};
