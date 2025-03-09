#pragma once
#include "RTFStreamFormatter.h"

class DefaultRTFFormatter : public RTFStreamFormatter
{
public:
    CStringA Format(UTF8Stream& stream) override;
    CStringA GetDefaultFormat() const override;
};

