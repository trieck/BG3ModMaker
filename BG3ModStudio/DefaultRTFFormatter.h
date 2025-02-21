#pragma once
#include "RTFStreamFormatter.h"

class DefaultRTFFormatter : public RTFStreamFormatter
{
public:
    CStringW Format(TextStream& stream) override;
};

