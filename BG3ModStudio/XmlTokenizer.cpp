#include "stdafx.h"
#include "XmlTokenizer.h"

CStringA XmlToken::GetValue() const
{
    return value;
}

XmlTokenType XmlToken::GetType() const
{
    return type;
}

CStringA XmlToken::GetTypeAsString() const
{
    switch (type) {
    case XmlTokenType::TT_EMPTY:
        return "Empty";
    case XmlTokenType::TT_TAG_START:
        return "Tag Start";
    case XmlTokenType::TT_TAG_END:
        return "Tag End";
    case XmlTokenType::TT_TAG_SELF_CLOSE:
        return "Tag Self Close";
    case XmlTokenType::TT_TAG_NAME:
        return "Tag Name";
    case XmlTokenType::TT_ATTRIBUTE_NAME:
        return "Attribute Name";
    case XmlTokenType::TT_ATTRIBUTE_VALUE:
        return "Attribute Value";
    case XmlTokenType::TT_TEXT:
        return "Text";
    case XmlTokenType::TT_COMMENT:
        return "Comment";
    case XmlTokenType::TT_CDATA:
        return "CDATA";
    case XmlTokenType::TT_DOCTYPE:
        return "DOCTYPE";
    case XmlTokenType::TT_INSTRUCTION_START:
        return "Instruction Start";
    case XmlTokenType::TT_INSTRUCTION_END:
        return "Instruction End";
    case XmlTokenType::TT_WHITESPACE:
        return "Whitespace";
    case XmlTokenType::TT_NEWLINE:
        return "Newline";
    case XmlTokenType::TT_QUOTE:
        return "Quote";
    case XmlTokenType::TT_EQUAL:
        return "Equal";
    case XmlTokenType::TT_ERROR:
        return "Error";
    default:
        return "Unknown";
    }
}

XmlTokenizer::XmlTokenizer() : m_input(nullptr), m_current(nullptr), m_isEnd(false), m_inTag(false),
                               m_inDoubleQuotes(false), m_inSingleQuotes(false)
{
}

XmlTokenizer::XmlTokenizer(LPCSTR input) : m_input(input), m_current(input), m_isEnd(false), m_inTag(false),
                                            m_inDoubleQuotes(false), m_inSingleQuotes(false)
{
}

void XmlTokenizer::SetInput(LPCSTR input)
{
    m_input = input;
    m_current = input;
    m_isEnd = false;
    m_inTag = false;
    m_inSingleQuotes = false;
    m_inDoubleQuotes = false;
}

XmlToken::Ptr XmlTokenizer::GetNextToken()
{
    if (m_isEnd) {
        return nullptr;
    }

    auto tok = GetToken(&m_current);
    if (tok.type == XmlTokenType::TT_EMPTY) {
        m_isEnd = true;
        return nullptr;
    }

    return std::make_unique<XmlToken>(tok);
}

void XmlTokenizer::Reset()
{
    m_current = m_input;
    m_isEnd = false;
}

bool XmlTokenizer::IsEnd() const
{
    return m_isEnd;
}

XmlToken XmlTokenizer::GetToken(LPCSTR* ppin)
{
    XmlToken tok;

    for (;;) {
        switch (**ppin) {
        case '\0':
            tok.type = XmlTokenType::TT_EMPTY;
            (*ppin)++;
            return tok;
        case '<':
            m_inTag = true;
            if (*(*ppin + 1) == '/') {
                tok.type = XmlTokenType::TT_TAG_END;
                tok.value = "</";
                *ppin += 2;
                return tok;
            }
            if (strcmp((*ppin) + 1, "!--") == 0) {
                tok.type = XmlTokenType::TT_COMMENT;
                tok.value = "<!--";
                *ppin += 4;

                // Read until "-->"
                while (**ppin && strncmp(*ppin, "-->", 3) != 0) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin) {
                    tok.value += "-->";
                    *ppin += 3;
                }

                return tok;
            }
            if (strcmp((*ppin) + 1, "![CDATA[") == 0) {
                tok.type = XmlTokenType::TT_CDATA;
                tok.value = "<![CDATA[";
                *ppin += 9;

                // Read until "]]>"
                while (**ppin && strncmp(*ppin, "]]>", 3) != 0) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin) {
                    tok.value += "]]>";
                    *ppin += 3;
                }

                return tok;
            }
            if (*(*ppin + 1) == '?') {
                tok.type = XmlTokenType::TT_INSTRUCTION_START;
                tok.value = "<?";
                *ppin += 2;
                return tok;
            }
            if (strncmp((*ppin) + 1, "!DOCTYPE", 9) == 0) {
                tok.type = XmlTokenType::TT_DOCTYPE;
                tok.value = "<!DOCTYPE";
                *ppin += 9;
                return tok;
            }
            tok.type = XmlTokenType::TT_TAG_START;
            tok.value = "<";
            *ppin += 1;
            return tok;
        case '>':
            m_inTag = false;
            tok.type = XmlTokenType::TT_TAG_END;
            tok.value = ">";
            (*ppin)++;
            return tok;
        case '/':
            if (*(*ppin + 1) == '>') {
                m_inTag = false;
                tok.type = XmlTokenType::TT_TAG_SELF_CLOSE;
                tok.value = "/>";
                *ppin += 2;
                return tok;
            }
            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        case '=':
            tok.type = XmlTokenType::TT_EQUAL;
            tok.value = "=";
            (*ppin)++;
            return tok;
        case '?':
            if (*(*ppin + 1) == '>') {
                m_inTag = false;
                tok.type = XmlTokenType::TT_INSTRUCTION_END;
                tok.value = "?>";
                *ppin += 2;
                return tok;
            }
            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        default:
            if (isspace(**ppin)) {
                while (_istspace(**ppin)) {
                    tok.value += *(*ppin)++;
                }
                tok.type = XmlTokenType::TT_WHITESPACE;
                return tok;
            }
            if (**ppin == '"') {
                if (m_inDoubleQuotes) {
                    m_inDoubleQuotes = false;
                    tok.type = XmlTokenType::TT_QUOTE;
                } else if (m_inTag && !m_inSingleQuotes) {
                    m_inDoubleQuotes = true;
                    tok.type = XmlTokenType::TT_QUOTE;
                } else {
                    tok.type = XmlTokenType::TT_TEXT;
                }

                tok.value = *(*ppin)++;
                return tok;
            }

            if (**ppin == '\'') {
                if (m_inSingleQuotes) {
                    m_inSingleQuotes = false;
                    tok.type = XmlTokenType::TT_QUOTE;
                } else if (m_inTag && !m_inDoubleQuotes) {
                    m_inSingleQuotes = true;
                    tok.type = XmlTokenType::TT_QUOTE;
                } else {
                    tok.type = XmlTokenType::TT_TEXT;
                }

                tok.value = *(*ppin)++;
                return tok;
            }

            if (**ppin == '\n' || **ppin == '\r') {
                tok.type = XmlTokenType::TT_NEWLINE;
                tok.value = *(*ppin)++;
                return tok;
            }
            if (isalnum(**ppin)) {
                while (isalnum(**ppin) || **ppin == '_' || **ppin == '-' || **ppin == '.' || **ppin ==
                    ':') {
                    tok.value += *(*ppin)++;
                }

                if (**ppin == '=') {
                    tok.type = XmlTokenType::TT_ATTRIBUTE_NAME;
                } else if (m_inSingleQuotes || m_inDoubleQuotes) {
                    tok.type = XmlTokenType::TT_ATTRIBUTE_VALUE;
                } else if (m_inTag) {
                    tok.type = XmlTokenType::TT_TAG_NAME;
                } else {
                    tok.type = XmlTokenType::TT_TEXT;
                }

                return tok;
            }

            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        }
    }
}
