#include "stdafx.h"
#include "XmlTokenizer.h"

XmlTokenizer::XmlTokenizer() : m_input(nullptr), m_current(nullptr)
{
}

XmlTokenizer::XmlTokenizer(LPCSTR input) : m_input(input), m_current(input)
{
}

uint32_t XmlTokenizer::GetState() const
{
    return m_state;
}

void XmlTokenizer::SetState(uint32_t state)
{
    ATLTRACE("SetState() called! Previous state: %d -> New state: %d\n", m_state, state);
    m_state = state;
}

void XmlTokenizer::SetInput(LPCSTR input)
{
    m_input = input;
    m_current = input;
}

XmlToken XmlTokenizer::GetNextToken()
{
    return GetToken(&m_current);
}

void XmlTokenizer::ResetState()
{
    ATLTRACE("ResetState() called! Previous state: %d\n", m_state);
    m_state = static_cast<uint32_t>(State::reset);
}

uint32_t XmlTokenizer::Recover()
{
    // This algorithm is designed to recover from an unknown state
    // and return to a known state. We need to be able to start parsing
    // in an unknown state, in the middle of an XML fragment and
    // recover to a known state.

    // This has MAJOR flaws as it cannot recover in the middle of a comment
    // When we encounter a normal state changing sequence, we fail to recover
    // properly if we are actually in a comment.

    // I see now what we must do!  We must REWIND to the first occurence of a start tag!
    // Reset to start tag state and move forward until we return back at our original input...
    // but it breaks down if start tags are inside comments! So...

    m_state = static_cast<uint32_t>(State::reset);

    m_current = m_input + strlen(m_input) - 1;

    auto* pin = m_current;

    while (pin > m_input) {
        switch (*pin) {
        case '\0':
            break;
        case '<':
            SetState(State::inTag); // start of tag
            break;
        case '>':   // end of tag, comment, CDATA, etc.
            SetState(State::reset);
            break;
        case '-':
            if (pin - 3 < m_input) {
                continue;   // not enough characters to check for comment
            }            
            if (strncmp(pin - 3, "<!-", 3) == 0) {  // in a comment
                SetState(State::inComment);
                pin -= 3;
                break;
            }
            break;
        case '[':
            if (pin - 6 < m_input) {
                continue;   // not enough characters to check for CDATA
            }
            if (strncmp(pin - 9, "<![CDATA[", 9) == 0) {  // in CDATA
                SetState(State::inCDATA);
                pin -= 9;
                break;
            }
            break;
        case '?':
            if (pin - 2 < m_input) {
                continue;   // not enough characters to check for processing instruction
            }
            if (strncmp(pin - 2, "<?", 2) == 0) {  // in processing instruction
                SetState(State::inProcInstr);
                pin -= 2;
                break;
            }
            break;
            // !DOCTYPE
        case 'E':
            if (pin - 8 < m_input) {
                continue;   // not enough characters to check for DOCTYPE
            }
            if (strncmp(pin - 8, "<!DOCTYPE", 8) == 0) {  // in DOCTYPE
                SetState(State::inDocType);
                pin -= 8;
                break;
            }
            break;
        case '"':
            // TODO:
            break;
        case '\'':
            // TODO:
            break;
        case '=':
            // TODO:
            break;
        default:
            break;
        }
    }

    m_current = pin;

    return m_state;
}

BOOL XmlTokenizer::HasState(uint32_t state) const
{
    return (m_state & state) != 0;
}

BOOL XmlTokenizer::HasState(State state) const
{
    return HasState(static_cast<uint32_t>(state));
}

void XmlTokenizer::AddState(uint32_t state)
{
    m_state |= state;
}

void XmlTokenizer::AddState(State state)
{
    AddState(static_cast<uint32_t>(state));
}

void XmlTokenizer::RemoveState(uint32_t state)
{
    m_state &= ~state;
}

void XmlTokenizer::RemoveState(State state)
{
    RemoveState(static_cast<uint32_t>(state));
}

void XmlTokenizer::SetState(State state)
{
    SetState(static_cast<uint32_t>(state));
}

XmlToken XmlTokenizer::GetToken(LPCSTR* ppin)
{
    XmlToken tok;

    for (;;) {
        if (**ppin == '\0') {
            tok.type = XmlTokenType::TT_EMPTY;
            //reset();
            return tok;
        }
        if (HasState(State::inComment)) {
            tok.type = XmlTokenType::TT_COMMENT;

            // Read until "-->"
            while (**ppin && strncmp(*ppin, "-->", 3) != 0) {
                tok.value += *(*ppin)++;
            }

            if (**ppin) {
                tok.value += "-->";
                ATLTRACE("Comment2 ended! : %s\n", tok.value.GetString());
                ResetState();
                *ppin += 3;
            }

            return tok;
        }
        if (HasState(State::inCDATA)) {
            tok.type = XmlTokenType::TT_CDATA;

            // Read until "]]>"
            while (**ppin && strncmp(*ppin, "]]>", 3) != 0) {
                tok.value += *(*ppin)++;
            }

            if (**ppin) {
                tok.value += "]]>";
                ResetState();
                *ppin += 3;
            }

            return tok;
        }
        if (HasState(State::inDocType)) {
            tok.type = XmlTokenType::TT_DOCTYPE;

            // Read until ">"
            while (**ppin && strncmp(*ppin, ">", 1) != 0) {
                tok.value += *(*ppin)++;
            }

            if (**ppin) {
                tok.value += ">";
                ResetState();
                *ppin += 1;
            }

            return tok;
        }

        if (HasState(State::inProcInstr)) {
            tok.type = XmlTokenType::TT_INSTRUCTION_START;
            if (strncmp(*ppin, "?>", 2) == 0) {
                tok.type = XmlTokenType::TT_INSTRUCTION_END;
                ResetState();
                tok.value = "?>";
                *ppin += 2;
                return tok;
            }
            tok.value += *(*ppin)++;
            return tok;
        }

        switch (**ppin) {
        case '\0':
            tok.type = XmlTokenType::TT_EMPTY;
            return tok;
        case '<':
            SetState(State::inTag);
            if (*(*ppin + 1) == '/') {
                tok.type = XmlTokenType::TT_TAG_END;
                tok.value = "</";
                *ppin += 2;
                return tok;
            }
            if (strncmp(*ppin + 1, "!--", 3) == 0) {
                SetState(State::inComment);
                tok.type = XmlTokenType::TT_COMMENT;
                tok.value = "<!--";
                *ppin += 4;

                // Read until "-->"
                while (**ppin && strncmp(*ppin, "-->", 3) != 0) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin) {
                    tok.value += "-->";
                    ATLTRACE("Comment1 ended! : %s\n", tok.value.GetString());
                    ResetState();
                    *ppin += 3;
                }

                 return tok;
            }
            if (strncmp(*ppin + 1, "![CDATA[", 8) == 0) {
                AddState(State::inCDATA);
                tok.type = XmlTokenType::TT_CDATA;
                tok.value = "<![CDATA[";
                *ppin += 9;

                // Read until "]]>"
                while (**ppin && strncmp(*ppin, "]]>", 3) != 0) {
                    tok.value += *(*ppin)++;
                }

                if (**ppin) {
                    tok.value += "]]>";
                    ResetState();
                    *ppin += 3;
                }

                return tok;
            }
            if (*(*ppin + 1) == '?') {
                AddState(State::inProcInstr);
                tok.type = XmlTokenType::TT_INSTRUCTION_START;
                tok.value = "<?";
                *ppin += 2;
                return tok;
            }
            if (strncmp(*ppin + 1, "!DOCTYPE", 8) == 0) {
                AddState(State::inDocType);
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
            ResetState();
            tok.type = XmlTokenType::TT_TAG_END;
            tok.value = ">";
            (*ppin)++;
            return tok;
        case '/':
            if (*(*ppin + 1) == '>') {
                ResetState();
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
                ResetState();
                tok.type = XmlTokenType::TT_INSTRUCTION_END;
                tok.value = "?>";
                *ppin += 2;
                return tok;
            }
            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        case '-':
            if (strncmp(*ppin, "-->", 3) == 0) {
                ResetState();
                tok.type = XmlTokenType::TT_COMMENT;
                tok.value = "-->";
                *ppin += 3;
                return tok;
            }
            tok.type = XmlTokenType::TT_UNKNOWN;
            tok.value = *(*ppin)++;
            return tok;
        default:
            if (isspace(**ppin)) {
                while (isspace(**ppin)) {
                    tok.value += *(*ppin)++;
                }
                tok.type = XmlTokenType::TT_WHITESPACE;
                return tok;
            }
            if (**ppin == '"') {
                if (HasState(State::inDQuote)) {
                    ResetState();
                    tok.type = XmlTokenType::TT_QUOTE;
                } else if (HasState(State::inTag) && !HasState(State::inSQuote)) {
                    AddState(State::inDQuote);
                    tok.type = XmlTokenType::TT_QUOTE;
                } else {
                    tok.type = XmlTokenType::TT_TEXT;
                }

                tok.value = *(*ppin)++;
                return tok;
            }

            if (**ppin == '\'') {
                if (HasState(State::inSQuote)) {
                    RemoveState(State::inSQuote);
                    tok.type = XmlTokenType::TT_QUOTE;
                } else if (HasState(State::inTag) && !HasState(State::inDQuote)) {
                    AddState(State::inSQuote);
                    tok.type = XmlTokenType::TT_QUOTE;
                } else {
                    tok.type = XmlTokenType::TT_TEXT;
                }

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
                } else if (HasState(State::inSQuote) && !HasState(State::inDQuote)) {
                    tok.type = XmlTokenType::TT_ATTRIBUTE_VALUE;
                } else if (HasState(State::inDQuote) && !HasState(State::inSQuote)) {
                    tok.type = XmlTokenType::TT_ATTRIBUTE_VALUE;
                } else if (HasState(State::inTag)) {
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
