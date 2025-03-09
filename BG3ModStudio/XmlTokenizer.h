#pragma once

#include "XmlToken.h"

class XmlTokenizer
{
public:
    XmlTokenizer();
    explicit XmlTokenizer(LPCSTR input);
    ~XmlTokenizer() = default;

    enum class State : uint32_t
    {
        reset = 0x00,
        inTag = 0x01,
        inAttrName = 0x02,
        inAtrrVal = 0x04,
        inDQuote = 0x8,
        inSQuote = 0x10,
        inComment = 0x20,
        inCDATA = 0x40,
        inProcInstr = 0x80,
        inDocType = 0x100,
    };

    uint32_t GetState() const;
    void SetState(uint32_t state);
    void SetInput(LPCSTR input);
    XmlToken GetNextToken();
    void ResetState();
    uint32_t Recover();

private:
    
    BOOL HasState(uint32_t state) const;
    BOOL HasState(State state) const;

    void AddState(uint32_t state);
    void AddState(State state);

    void RemoveState(uint32_t state);
    void RemoveState(State state);

    void SetState(State state);

    XmlToken GetToken(LPCSTR* ppin);

    uint32_t m_state{0};
    LPCSTR m_input;
    LPCSTR m_current;
};
