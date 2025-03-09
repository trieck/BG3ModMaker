#pragma once

#include "RTFStreamFormatter.h"
#include "XmlTokenizer.h"

class XmlRTFFormatter : public RTFStreamFormatter
{
public:
    CStringA Format(UTF8Stream& stream) override;
    CStringA GetDefaultFormat() const override;

private:
    XmlTokenizer m_tokenizer;
};
