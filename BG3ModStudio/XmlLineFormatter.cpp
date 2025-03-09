#include "stdafx.h"
#include "StringHelper.h"
#include "XmlFormat.h"
#include "XmlLineFormatter.h"
#include "XmlTokenizer.h"

FormatTokenVec XmlLineFormatter::Format(const CString& line)
{
    FormatTokenVec tokens;

    auto utf8Line = StringHelper::toUTF8(line);

    m_tokenizer.SetInput(utf8Line);

    LONG start = 0;
    
    for (;;) {
        auto token = m_tokenizer.GetNextToken();
        auto type = token.GetType();
        if (type == XmlTokenType::TT_EMPTY) {
            break;
        }

        if (type != XmlTokenType::TT_UNKNOWN && type != XmlTokenType::TT_EMPTY) {
            auto color = XmlGetColor(type);

            FormatToken fmtToken;
            fmtToken.start = start;
            fmtToken.end = start + token.GetValue().GetLength();
            fmtToken.color = XmlToColorRef(color);
            tokens.emplace_back(fmtToken);
        }

        start += token.GetValue().GetLength();
    }
    
    return tokens;
}

uint32_t XmlLineFormatter::GetState() const
{
    return m_tokenizer.GetState();
}

void XmlLineFormatter::SetState(uint32_t state)
{
    m_tokenizer.SetState(state);
}

void XmlLineFormatter::Reset()
{
    m_tokenizer.ResetState();
}
