#pragma once

#include "ScintillaCtrl.h"
#include "UtilityBase.h"

class DocStyler
{
public:
    using Ptr = std::shared_ptr<DocStyler>;
    virtual void Apply(ScintillaCtrl& ctrl) = 0;
    virtual ~DocStyler() = default;
};

class DocStylerRegistry
{
public:
    static DocStylerRegistry& GetInstance();
    static DocStyler::Ptr GetStyler(const CString& path);
    static DocStyler::Ptr GetDefaultStyler();

private:
    DocStylerRegistry();
    std::unordered_map<std::string, std::shared_ptr<DocStyler>> m_stylers;
    std::shared_ptr<DocStyler> m_default;
};
