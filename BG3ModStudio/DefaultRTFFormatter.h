#pragma once
#include "IRTFStreamFormatter.h"

class DefaultRTFFormatter : public IRTFStreamFormatter
{
public:
    CStringW Format(TextStream& stream) override;
};

