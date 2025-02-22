#pragma once

#include "RTFStreamFormatter.h"
#include "TextTokenizer.h"

class TextRTFFormatter : public RTFStreamFormatter
{
public:
    CStringA Format(UTF8Stream& stream) override;

private:
    TextTokenizer m_tokenizer;
};

