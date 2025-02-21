#include "stdafx.h"
#include "XmlTokenizer.h"

CString XmlToken::GetValue() const
{
    return value;
}

XmlTokenType XmlToken::GetType() const
{
    return type;
}

CString XmlToken::GetTypeAsString() const
{
    switch (type) {
    case XmlTokenType::TT_EMPTY:
        return _T("Empty");
    case XmlTokenType::TT_TAG_START:
        return _T("Tag Start");
    case XmlTokenType::TT_TAG_END:
        return _T("Tag End");
    case XmlTokenType::TT_TAG_SELF_CLOSE:
        return _T("Tag Self Close");
    case XmlTokenType::TT_TAG_NAME:
        return _T("Tag Name");
    case XmlTokenType::TT_ATTRIBUTE_NAME:
        return _T("Attribute Name");
    case XmlTokenType::TT_ATTRIBUTE_VALUE:
        return _T("Attribute Value");
    case XmlTokenType::TT_TEXT:
        return _T("Text");
    case XmlTokenType::TT_COMMENT:
        return _T("Comment");
    case XmlTokenType::TT_CDATA:
        return _T("CDATA");
    case XmlTokenType::TT_DOCTYPE:
        return _T("DOCTYPE");
    case XmlTokenType::TT_INSTRUCTION_START:
        return _T("Instruction Start");
    case XmlTokenType::TT_INSTRUCTION_END:
        return _T("Instruction End");
    case XmlTokenType::TT_WHITESPACE:
        return _T("Whitespace");
    case XmlTokenType::TT_NEWLINE:
        return _T("Newline");
    case XmlTokenType::TT_QUOTE:
        return _T("Quote");
    case XmlTokenType::TT_EQUAL:
        return _T("Equal");
    case XmlTokenType::TT_ERROR:
        return _T("Error");
    default:
        return _T("Unknown");
    }
}

XmlTokenizer::XmlTokenizer() : m_input(nullptr), m_current(nullptr), m_isEnd(false), m_inTag(false), m_inQuote(false)
{
}

XmlTokenizer::XmlTokenizer(LPCTSTR input) : m_input(input), m_current(input), m_isEnd(false), m_inTag(false),
                                            m_inQuote(false)
{
}

void XmlTokenizer::SetInput(LPCTSTR input)
{
    m_input = input;
    m_current = input;
    m_isEnd = false;
    m_inTag = false;
    m_inQuote = false;
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

XmlToken XmlTokenizer::GetToken(LPCTSTR* ppin)
{
    XmlToken tok;

    for (;;) {
        switch (**ppin) {
        case _T('\0'):
            tok.type = XmlTokenType::TT_EMPTY;
            (*ppin)++;
            return tok;
        case _T('<'):
            m_inTag = true;
            if (*(*ppin + 1) == _T('/')) {
                tok.type = XmlTokenType::TT_TAG_END;
                tok.value = _T("</");
                *ppin += 2;
                return tok;
            }
            if (_tcscmp((*ppin) + 1, _T("!--")) == 0) {
                tok.type = XmlTokenType::TT_COMMENT;
                tok.value = _T("<!--");
                *ppin += 4;

                // Read until "-->"
                while (**ppin && _tcsncmp(*ppin, _T("-->"), 3) != 0) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin) {
                    tok.value += _T("-->");
                    *ppin += 3;
                }

                return tok;
            }
            if (_tcscmp((*ppin) + 1, _T("![CDATA[")) == 0) {
                tok.type = XmlTokenType::TT_CDATA;
                tok.value = _T("<![CDATA[");
                *ppin += 9;

                // Read until "]]>"
                while (**ppin && _tcsncmp(*ppin, _T("]]>"), 3) != 0) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin) {
                    tok.value += _T("]]>");
                    *ppin += 3;
                }

                return tok;
            }
            if (*(*ppin + 1) == _T('?')) {
                tok.type = XmlTokenType::TT_INSTRUCTION_START;
                tok.value = _T("<?");
                *ppin += 2;
                return tok;
            }
            if (_tcsncmp((*ppin) + 1, _T("!DOCTYPE"), 9) == 0) {
                tok.type = XmlTokenType::TT_DOCTYPE;
                tok.value = _T("<!DOCTYPE");
                *ppin += 9;
                return tok;
            }
            tok.type = XmlTokenType::TT_TAG_START;
            tok.value = _T("<");
            *ppin += 1;
            return tok;
        case _T('>'):
            m_inTag = false;
            tok.type = XmlTokenType::TT_TAG_END;
            tok.value = _T(">");
            (*ppin)++;
            return tok;
        case _T('/'):
            if (*(*ppin + 1) == _T('>')) {
                m_inTag = false;
                tok.type = XmlTokenType::TT_TAG_SELF_CLOSE;
                tok.value = _T("/>");
                *ppin += 2;
                return tok;
            }
            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        case _T('='):
            tok.type = XmlTokenType::TT_EQUAL;
            tok.value = _T("=");
            (*ppin)++;
            return tok;
        case _T('?'):
            if (*(*ppin + 1) == _T('>')) {
                m_inTag = false;
                tok.type = XmlTokenType::TT_INSTRUCTION_END;
                tok.value = _T("?>");
                *ppin += 2;
                return tok;
            }
            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        default:
            if (_istspace(**ppin)) {
                while (_istspace(**ppin)) {
                    tok.value += *(*ppin)++;
                }
                tok.type = XmlTokenType::TT_WHITESPACE;
                return tok;
            }
            if (**ppin == _T('"') || **ppin == _T('\'')) {
                m_inQuote = !m_inQuote;
                tok.type = XmlTokenType::TT_QUOTE;
                tok.value = *(*ppin)++;
                return tok;
            }
            if (**ppin == _T('\n') || **ppin == _T('\r')) {
                tok.type = XmlTokenType::TT_NEWLINE;
                tok.value = *(*ppin)++;
                return tok;
            }
            if (_istalnum(**ppin)) {
                while (_istalnum(**ppin) || **ppin == _T('_') || **ppin == _T('-') || **ppin == _T('.') || **ppin ==
                    _T(':')) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin == _T('=')) {
                    tok.type = XmlTokenType::TT_ATTRIBUTE_NAME;
                } else if (m_inQuote) {
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
