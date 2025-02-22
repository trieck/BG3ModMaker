#include "stdafx.h"
#include "JsonTokenizer.h"

CStringA JsonToken::GetValue() const
{
    return value;
}

JsonTokenType JsonToken::GetType() const
{
    return type;
}

CStringA JsonToken::GetTypeAsString() const
{
    switch (type) {
    case JsonTokenType::TT_EMPTY:
        return "Empty";
    case JsonTokenType::TT_OBJECT_START:
        return "Object Start";
    case JsonTokenType::TT_OBJECT_END:
        return "Object End";
    case JsonTokenType::TT_ARRAY_START:
        return "Array Start";
    case JsonTokenType::TT_ARRAY_END:
        return "Array End";
    case JsonTokenType::TT_COMMA:
        return "Comma";
    case JsonTokenType::TT_COLON:
        return "Colon";
    case JsonTokenType::TT_STRING:
        return "String";
    case JsonTokenType::TT_NUMBER:
        return "Number";
    case JsonTokenType::TT_BOOLEAN:
        return "Boolean";
    case JsonTokenType::TT_NULL:
        return "Null";
    case JsonTokenType::TT_WHITESPACE:
        return "Whitespace";
    case JsonTokenType::TT_NEWLINE:
        return "Newline";
    case JsonTokenType::TT_ERROR:
        return "Error";
    default:
        return "Unknown";
    }
}

JsonTokenizer::JsonTokenizer() : m_input(nullptr), m_current(nullptr), m_isEnd(false)
{
}

JsonTokenizer::JsonTokenizer(LPCSTR input) : m_input(input), m_current(input), m_isEnd(false)
{
}

void JsonTokenizer::SetInput(LPCSTR input)
{
    m_input = input;
    m_current = input;
    m_isEnd = false;
}

JsonToken::Ptr JsonTokenizer::GetNextToken()
{
    if (m_isEnd) {
        return nullptr;
    }

    auto tok = GetToken(&m_current);
    if (tok.type == JsonTokenType::TT_EMPTY) {
        m_isEnd = true;
        return nullptr;
    }

    return std::make_unique<JsonToken>(tok);
}

void JsonTokenizer::Reset()
{
    m_current = m_input;
    m_isEnd = false;
}

bool JsonTokenizer::IsEnd() const
{
    return m_isEnd;
}

JsonToken JsonTokenizer::GetToken(LPCSTR* ppin)
{
    JsonToken tok;

    for (;;) {
        switch (**ppin) {
        case '\0':
            tok.type = JsonTokenType::TT_EMPTY;
            return tok;
        case '{':
            tok.type = JsonTokenType::TT_OBJECT_START;
            tok.value = *(*ppin)++;
            return tok;
        case '}':
            tok.type = JsonTokenType::TT_OBJECT_END;
            tok.value = *(*ppin)++;
            return tok;
        case '[':
            tok.type = JsonTokenType::TT_ARRAY_START;
            tok.value = *(*ppin)++;
            return tok;
        case ']':
            tok.type = JsonTokenType::TT_ARRAY_END;
            tok.value = *(*ppin)++;
            return tok;
        case ',':
            tok.type = JsonTokenType::TT_COMMA;
            tok.value = *(*ppin)++;
            return tok;
        case ':':
            tok.type = JsonTokenType::TT_COLON;
            tok.value = *(*ppin)++;
            return tok;
        case '"':
            tok.type = JsonTokenType::TT_STRING;
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
        case 't':
            if (strncmp(*ppin, "true", 4) == 0) {
                tok.type = JsonTokenType::TT_BOOLEAN;
                tok.value = "true";
                *ppin += 4;
                return tok;
            }

            tok.value = *(*ppin)++;
            tok.type = JsonTokenType::TT_ERROR;
            return tok;
        case 'f':
            if (strncmp(*ppin, "false", 5) == 0) {
                tok.type = JsonTokenType::TT_BOOLEAN;
                tok.value = "false";
                *ppin += 5;
                return tok;
            }
            tok.value = *(*ppin)++;
            tok.type = JsonTokenType::TT_ERROR;
            return tok;
        case 'n':
            if (strncmp(*ppin, "null", 4) == 0) {
                tok.type = JsonTokenType::TT_NULL;
                tok.value = "null";
                *ppin += 4;
                return tok;
            }
            tok.value = *(*ppin)++;
            tok.type = JsonTokenType::TT_ERROR;
            return tok;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            while (isspace(**ppin)) {
                tok.value += *(*ppin)++;
            }
            tok.type = JsonTokenType::TT_WHITESPACE;
            return tok;
        default:
            if (isdigit(**ppin) || **ppin == '-') {
                tok.type = JsonTokenType::TT_NUMBER;
                while (isdigit(**ppin) || **ppin == '.' || **ppin == 'e' || **ppin == 'E' || **ppin == '+' || **ppin == '-') {
                    tok.value += *(*ppin)++;
                }
                return tok;
            }
            tok.value = *(*ppin)++;
            tok.type = JsonTokenType::TT_ERROR;
            return tok;
        }
    }
}
