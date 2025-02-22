#include "stdafx.h"
#include "TextTokenizer.h"

CStringA TextToken::GetValue() const
{
    return value;
}

TextTokenType TextToken::GetType() const
{
    return type;
}

CStringA TextToken::GetTypeAsString() const
{
    switch (type) {
    case TextTokenType::TT_EMPTY:
        return "Empty";
    case TextTokenType::TT_WORD:
        return "Word";
    case TextTokenType::TT_NUMBER:
        return "Number";
    case TextTokenType::TT_QUOTED_STRING:
        return "Quoted String";
    case TextTokenType::TT_WHITESPACE:
        return "Whitespace";
    default:
        return "Unknown";
    }
}

TextTokenizer::TextTokenizer() : m_input(nullptr), m_current(nullptr), m_isEnd(false)
{
}

TextTokenizer::TextTokenizer(LPCSTR input)
{
    SetInput(input);
}

void TextTokenizer::SetInput(LPCSTR input)
{
    m_input = input;
    m_current = input;
    m_isEnd = false;
}

TextToken::Ptr TextTokenizer::GetNextToken()
{
    if (m_isEnd) {
        return nullptr;
    }

    auto tok = GetToken(&m_current);
    if (tok.type == TextTokenType::TT_EMPTY) {
        m_isEnd = true;
        return nullptr;
    }

    return std::make_unique<TextToken>(tok);
}

void TextTokenizer::Reset()
{
    m_current = m_input;
    m_isEnd = false;
}

bool TextTokenizer::IsEnd() const
{
    return m_isEnd;
}

TextToken TextTokenizer::GetToken(LPCSTR* ppin)
{
    TextToken tok;

    for (;;) {
        switch (**ppin) {
        case '\0':
            tok.type = TextTokenType::TT_EMPTY;
            return tok;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            while (isspace(**ppin)) {
                tok.value += *(*ppin)++;
            }
            tok.type = TextTokenType::TT_WHITESPACE;
            return tok;
        case '"':
            tok.type = TextTokenType::TT_QUOTED_STRING;
            tok.value = *(*ppin)++;
            while (**ppin && **ppin != '"') {
                if (**ppin == '\\') {  // Handle escape sequences
                    tok.value += *(*ppin)++; // Add '\'
                    if (**ppin) {
                        tok.value += *(*ppin)++; // Add next char
                    }
                } else {
                    tok.value += *(*ppin)++;
                }
            }
            if (**ppin) {
                tok.value += *(*ppin)++; //  '"'
            }
            return tok;
        default:
            if (isalpha(**ppin) || **ppin == '_') {
                tok.type = TextTokenType::TT_WORD;
                tok.value = *(*ppin)++;
                while (isalnum(**ppin) || **ppin == '_') {
                    tok.value += *(*ppin)++;
                }
                return tok;
            }
            if (isdigit(**ppin)) {
                tok.type = TextTokenType::TT_NUMBER;
                tok.value = *(*ppin)++;
                while (isdigit(**ppin) || **ppin == '.') {
                    tok.value += *(*ppin)++;
                }
                return tok;
            }
            tok.value = *(*ppin)++;
            tok.type = TextTokenType::TT_UNKNOWN;
            return tok;
        }
    }
}
