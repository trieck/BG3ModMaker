#pragma once

enum class JsonTokenType
{
    TT_UNKNOWN = -1,
    TT_EMPTY,
    TT_OBJECT_START,
    TT_OBJECT_END,
    TT_ARRAY_START,
    TT_ARRAY_END,
    TT_COMMA,
    TT_COLON,

    TT_STRING, // "string"
    TT_NUMBER, // 123, -45.67, 0.12e3
    TT_BOOLEAN, // true, false
    TT_NULL, // null

    TT_WHITESPACE,
    TT_NEWLINE,

    TT_ERROR // Invalid token
};

struct JsonToken
{
    JsonToken() = default;
    JsonToken(const JsonToken&) = default;

    using Ptr = std::unique_ptr<JsonToken>;

    CStringA GetValue() const;
    JsonTokenType GetType() const;
    CStringA GetTypeAsString() const;

    JsonTokenType type{ JsonTokenType::TT_EMPTY };
    CStringA value;
};

class JsonTokenizer
{
public:
    JsonTokenizer();
    explicit JsonTokenizer(LPCSTR input);
    ~JsonTokenizer() = default;

    void SetInput(LPCSTR input);

    JsonToken::Ptr GetNextToken();
    void Reset();
    bool IsEnd() const;

private:
    static JsonToken GetToken(LPCSTR* ppin);
    LPCSTR m_input;
    LPCSTR m_current;
    bool m_isEnd;
};
