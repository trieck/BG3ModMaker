#pragma once

enum class XmlTokenType
{
    TT_UNKNOWN = -1,
    TT_EMPTY,
    TT_TAG_START,
    TT_TAG_END,
    TT_TAG_SELF_CLOSE,
    TT_TAG_NAME,
    TT_ATTRIBUTE_NAME,
    TT_ATTRIBUTE_VALUE,
    TT_TEXT,
    TT_COMMENT,
    TT_CDATA,
    TT_DOCTYPE,
    TT_INSTRUCTION_START,
    TT_INSTRUCTION_END,
    TT_WHITESPACE,
    TT_NEWLINE,
    TT_QUOTE,
    TT_EQUAL,
    TT_ERROR
};

struct XmlToken
{
    XmlToken() = default;
    XmlToken(const XmlToken&) = default;

    using Ptr = std::unique_ptr<XmlToken>;

    CString GetValue() const;
    XmlTokenType GetType() const;
    CString GetTypeAsString() const;

    XmlTokenType type;
    CString value;
};

class XmlTokenizer
{
public:
    XmlTokenizer();
    explicit XmlTokenizer(LPCTSTR input);
    ~XmlTokenizer() = default;

    void SetInput(LPCTSTR input);

    XmlToken::Ptr GetNextToken();
    void Reset() ;
    bool IsEnd() const;

private:
    XmlToken GetToken(LPCTSTR* ppin);

    LPCTSTR m_input;
    LPCTSTR m_current;
    bool m_isEnd, m_inTag, m_inDoubleQuotes, m_inSingleQuotes;
};


