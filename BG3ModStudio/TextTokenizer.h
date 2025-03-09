#pragma once

enum class TextTokenType
{
    TT_UNKNOWN = -1,
    TT_EMPTY,
    TT_WORD,
    TT_NUMBER,
    TT_QUOTED_STRING,
    TT_WHITESPACE
};

struct TextToken
{
    TextToken() = default;
    TextToken(const TextToken&) = default;

    using Ptr = std::unique_ptr<TextToken>;

    CStringA GetValue() const;
    TextTokenType GetType() const;
    CStringA GetTypeAsString() const;

    TextTokenType type{ TextTokenType::TT_EMPTY };
    CStringA value;
};

class TextTokenizer
{
public:
    TextTokenizer();
    explicit TextTokenizer(LPCSTR input);
    ~TextTokenizer() = default;

    void SetInput(LPCSTR input);

    TextToken::Ptr GetNextToken();

    void Reset();
    bool IsEnd() const;

private:
    static TextToken GetToken(LPCSTR* ppin);
    LPCSTR m_input;
    LPCSTR m_current;
    bool m_isEnd;
};
