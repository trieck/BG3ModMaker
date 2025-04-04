#pragma once

#include "ScintillaCtrl.h"

class DocStyler
{
public:
    using Ptr = std::shared_ptr<DocStyler>;
    virtual void Apply(ScintillaCtrl& ctrl) = 0;
    virtual ~DocStyler() = default;
};

class XmlStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

class JsonStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

class PlainTextStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

class DocStylerRegistry
{
public:
    static DocStylerRegistry& GetInstance();
    DocStyler::Ptr GetStyler(const CString& path) const;
    DocStyler::Ptr GetDefaultStyler() const;

private:
    DocStylerRegistry();
    std::unordered_map<std::string, std::shared_ptr<DocStyler>> m_stylers;
    std::shared_ptr<DocStyler> m_default;
};
