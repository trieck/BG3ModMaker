#pragma once

#include "JSONTokenizer.h"
#include "RTFStreamFormatter.h"

class JSONRTFFormatter : public RTFStreamFormatter
{
public:
    CStringA Format(UTF8Stream& stream) override;

private:
    JsonTokenizer m_tokenizer;
};

