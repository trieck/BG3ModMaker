#pragma once

#include "UTF8Stream.h"

class RTFStreamFormatter
{
public:
    RTFStreamFormatter() = default;
    virtual ~RTFStreamFormatter() = default;

    using Ptr = std::shared_ptr<RTFStreamFormatter>;

    virtual CStringA Format(UTF8Stream& stream) = 0;
    virtual CStringA GetDefaultFormat() const = 0;

    static CStringA EscapeRTF(const CStringA& input);
    static CStringA GetRTFColor(COLORREF color);
};
