#pragma once

#include "IRTFStreamFormatter.h"

class RTFFormatterRegistry
{
public:
    RTFFormatterRegistry();
    ~RTFFormatterRegistry() = default;

    static RTFFormatterRegistry& GetInstance();

    IRTFStreamFormatter::Ptr GetFormatter(const CString& path) const;

private:
    std::unordered_map<std::string, IRTFStreamFormatter::Ptr> m_formatters;
    IRTFStreamFormatter::Ptr m_defaultFormatter;
};

