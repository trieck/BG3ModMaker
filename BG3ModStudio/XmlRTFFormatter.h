#pragma once

#include "RTFStreamFormatter.h"
#include "XmlTokenizer.h"

class XmlRTFFormatter : public RTFStreamFormatter
{
public:
    CStringW Format(TextStream& stream) override;

private:
    XmlTokenizer m_tokenizer;
};

