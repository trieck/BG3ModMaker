#pragma once

#include "IRTFStreamFormatter.h"
#include "XmlTokenizer.h"

class XmlRTFFormatter : public IRTFStreamFormatter
{
public:
    CStringW Format(TextStream& stream) override;

private:
    XmlTokenizer m_tokenizer;
};

