#pragma once

#include "TextStream.h"

class IRTFStreamFormatter
{
public:
    IRTFStreamFormatter() = default;
    virtual ~IRTFStreamFormatter() = default;

    using Ptr = std::shared_ptr<IRTFStreamFormatter>;

    virtual CStringW Format(TextStream& stream) = 0;
    static CStringW EscapeRTF(const CStringW& input);
};
