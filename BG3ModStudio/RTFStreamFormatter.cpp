#include "stdafx.h"
#include "RTFStreamFormatter.h"

namespace { // anonymous 
    struct Token
    {
        enum class Type
        {
            EMPTY = 0,
            ID,
            LITERAL,
            COMMENT,
            WHITESPACE,
            NEWLINE,
            OTHER
        };

        Token() : type(Type::EMPTY)
        {
        }

        Type type;
        CString value;
    };

    enum COLORS
    {
        COLOR_TEXT = 1,
        COLOR_KEYWORD,
        COLOR_PROPERTY,
        COLOR_COMMENT,
        COLOR_LITERAL,
        COLOR_TYPE,
    };

    BOOL ParseGUID(LPCTSTR pin)
    {
        ATLASSERT(pin);

        for (auto i = 0; i < 8; ++i) {
            if (!isxdigit(*pin++)) {
                return FALSE;
            }
        }

        if (*pin++ != '-') {
            return FALSE;
        }

        for (auto i = 0; i < 4; ++i) {
            if (!isxdigit(*pin++)) {
                return FALSE;
            }
        }

        if (*pin++ != '-') {
            return FALSE;
        }

        for (auto i = 0; i < 4; ++i) {
            if (!isxdigit(*pin++)) {
                return FALSE;
            }
        }

        if (*pin++ != '-') {
            return FALSE;
        }

        for (auto i = 0; i < 4; ++i) {
            if (!isxdigit(*pin++)) {
                return FALSE;
            }
        }

        if (*pin++ != '-') {
            return FALSE;
        }

        for (auto i = 0; i < 12; ++i) {
            if (!isxdigit(*pin++)) {
                return FALSE;
            }
        }

        return TRUE;
    }

    Token GetToken(LPCTSTR* ppin)
    {
        ATLASSERT(ppin);

        for (Token token;;) {
            switch (**ppin) {
            case '\0':
                token.type = Token::Type::EMPTY;
                return token;
            case '\r':
            case '\t':
            case ' ':
                while (_istspace(**ppin)) {
                    token.value += *(*ppin)++;
                }
                token.type = Token::Type::WHITESPACE;
                return token;
            case '/':
                if ((*ppin)[1] == '/') {
                    while (**ppin != '\0' && **ppin != '\n') {
                        token.value += *(*ppin)++;
                    }
                    token.type = Token::Type::COMMENT;
                } else if ((*ppin)[1] == '*') {
                    while (**ppin != '\0') {
                        token.value += **ppin;
                        if (**ppin == '/' && (*ppin)[-1] == '*') {
                            break;
                        }
                        (*ppin)++;
                    }
                    token.type = Token::Type::COMMENT;
                } else {
                    token.value = *(*ppin)++;
                    token.type = Token::Type::ID;
                }
                return token;
            case '"':
                token.value = *(*ppin)++;
                while (**ppin != '\0' && **ppin != '\n') {
                    token.value += *(*ppin)++;
                    if ((*ppin)[-1] == '"') {
                        break;
                    }
                }
                token.type = Token::Type::LITERAL;
                return token;
            case '\n':
                token.value = *(*ppin)++;
                token.type = Token::Type::NEWLINE;
                return token;
            default:
                if (isdigit(**ppin) || isxdigit(**ppin)) {
                    if (ParseGUID(*ppin)) {
                        token.value = CString(*ppin, 36);
                        *ppin += 36;
                        token.type = Token::Type::LITERAL;
                        return token;
                    }

                    if ((*ppin)[0] == '0' && ((*ppin)[1] == 'x' || (*ppin)[1] == 'X')) {
                        token.value += *(*ppin)++; // hex number
                        token.value += *(*ppin)++;
                        while (isxdigit(**ppin)) {
                            token.value += *(*ppin)++;
                        }
                        token.type = Token::Type::LITERAL;
                        return token;
                    }
                    if (isdigit(**ppin)) {
                        while (isdigit(**ppin) || **ppin == '.') {
                            token.value += *(*ppin)++;
                        }
                        token.type = Token::Type::LITERAL;
                        return token;
                    }
                    // fall through
                }
                if (iscsym(**ppin)) {
                    while (iscsym(**ppin)) {
                        token.value += *(*ppin)++;
                    }

                    // pointer identifier
                    while (**ppin == '*') {
                        token.value += *(*ppin)++;
                    }

                    token.type = Token::Type::ID;
                    return token;
                }

                if (ispunct(**ppin)) {
                    token.value = *(*ppin)++;
                    token.type = Token::Type::OTHER;
                    return token;
                }

                (*ppin)++; // eat
                break;
            }
        }
    }
} // anonymous

CStringA RTFStreamFormatter::Format(TextStream& stream)
{
    CString output(_T("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}"
        "{\\colortbl;"
        "\\red0\\green0\\blue0;" // COLOR_TEXT
        "\\red0\\green0\\blue200;" // COLOR_KEYWORD
        "\\red200\\green0\\blue0;" // COLOR_PROPERTY
        "\\red0\\green128\\blue0;" // COLOR_COMMENT
        "\\red200\\green0\\blue200;" // COLOR_LITERAL
        "\\red128\\green0\\blue0;" // COLOR_TYPE
        ";}\n"
        "\\cf1\n"));

    //  TODO: implement

    auto strText = stream.ReadString();

    LPCTSTR pin = strText;
    while (*pin) {

        switch (*pin) {
        case '\r':
            break;
        case '\n':
            output += _T("\\line\n");
            break;
        default:
            output += *pin;
            break;
        }

        pin++;
    }

    output += '}';

    CT2CA utf8Str(output, CP_UTF8);

    return CStringA(utf8Str);
}
