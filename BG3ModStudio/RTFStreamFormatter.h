#pragma once

#include "TextStream.h"

class RTFStreamFormatter
{
public:
    RTFStreamFormatter() = default;
    virtual ~RTFStreamFormatter() = default;

    using Ptr = std::shared_ptr<RTFStreamFormatter>;

    virtual CStringW Format(TextStream& stream) = 0;
    static CStringW EscapeRTF(const CStringW& input);
};
