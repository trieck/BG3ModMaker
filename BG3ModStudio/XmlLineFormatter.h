#pragma once

#include "FormatToken.h"
#include "XmlTokenizer.h"

class XmlLineFormatter
{
public:
    XmlLineFormatter() = default;
    virtual ~XmlLineFormatter() = default;
    using Ptr = std::shared_ptr<XmlLineFormatter>;

    FormatTokenVec Format(const CString& line);
    uint32_t GetState() const;
    void SetState(uint32_t state);
    void Reset();
private:
    XmlTokenizer m_tokenizer;
};
