#include "stdafx.h"
#include "DocStyler.h"
#include "lexilla/include/SciLexer.h"
#include "StringHelper.h"

class JsonStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

class LuaStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

class PlainTextStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

class XmlStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};

void JsonStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetLexer("json");
    ctrl.SetIndent(2);
    ctrl.SetTabWidth(2);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);
    ctrl.SetMarginType(0, SC_MARGIN_NUMBER);
    ctrl.SetMarginWidth(0, 40);

    ctrl.SetForeStyle(SCE_JSON_PROPERTYNAME, RGB(0, 0, 255));
    ctrl.SetForeStyle(SCE_JSON_STRING, RGB(163, 21, 21));
    ctrl.SetForeStyle(SCE_JSON_NUMBER, RGB(43, 145, 175));
    ctrl.SetForeStyle(SCE_JSON_OPERATOR, RGB(0, 0, 0));
}

void LuaStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetLexer("lua");
    ctrl.SetIndent(2);
    ctrl.SetTabWidth(2);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);
    ctrl.SetMarginType(0, SC_MARGIN_NUMBER);
    ctrl.SetMarginWidth(0, 40);

    ctrl.SetKeywords(0,
                     "and break do else elseif end false for function goto if in local nil not or repeat return then true until while");
    ctrl.SetKeywords(1,
                     "assert collectgarbage dofile error _G getmetatable ipairs load loadfile next pairs pcall print rawequal "
                     "rawget rawlen rawset require select setmetatable tonumber tostring type xpcall "
                     "coroutine debug io math os string table utf8");

    ctrl.SetForeStyle(SCE_LUA_COMMENT, RGB(0, 128, 0));
    ctrl.SetForeStyle(SCE_LUA_COMMENTDOC, RGB(0, 128, 0));
    ctrl.SetForeStyle(SCE_LUA_COMMENTLINE, RGB(0, 128, 0));
    ctrl.SetForeStyle(SCE_LUA_DEFAULT, RGB(0, 0, 64));
    ctrl.SetForeStyle(SCE_LUA_WORD, RGB(64, 128, 255)); // core keywords
    ctrl.SetForeStyle(SCE_LUA_WORD2, RGB(128, 0, 128)); // Lua standard library
    ctrl.SetForeStyle(SCE_LUA_STRING, RGB(163, 21, 21)); // red strings
    ctrl.SetForeStyle(SCE_LUA_LITERALSTRING, RGB(255, 64, 64)); // red long strings
    ctrl.SetForeStyle(SCE_LUA_LABEL, RGB(255, 128, 0)); // orange labels
    ctrl.SetForeStyle(SCE_LUA_IDENTIFIER, RGB(0, 0, 128)); // blue identifiers
    ctrl.SetForeStyle(SCE_LUA_DEFAULT, RGB(64, 128, 255)); // light blue default text
}

void XmlStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetLexer("xml");
    ctrl.SetIndent(2);
    ctrl.SetTabWidth(2);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);
    ctrl.SetMarginType(0, SC_MARGIN_NUMBER);
    ctrl.SetMarginWidth(0, 40);

    ctrl.SetForeStyle(SCE_H_TAG, RGB(163, 21, 21));
    ctrl.SetForeStyle(SCE_H_ATTRIBUTE, RGB(0, 0, 255));
    ctrl.SetForeStyle(SCE_H_DOUBLESTRING, RGB(163, 21, 21));
    ctrl.SetForeStyle(SCE_H_COMMENT, RGB(0, 128, 0));
}

void PlainTextStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetIndent(2);
    ctrl.SetTabWidth(2);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);
    ctrl.SetMarginType(0, SC_MARGIN_NUMBER);
    ctrl.SetMarginWidth(0, 40);
}

DocStylerRegistry& DocStylerRegistry::GetInstance()
{
    static DocStylerRegistry instance;
    return instance;
}

DocStylerRegistry::DocStylerRegistry()
{
    m_stylers[".lua"] = std::make_unique<LuaStyler>();
    m_stylers[".json"] = std::make_unique<JsonStyler>();
    m_stylers[".lsj"] = m_stylers[".json"];
    m_stylers[".js"] = m_stylers[".json"];
    m_stylers[".txt"] = std::make_unique<PlainTextStyler>();
    m_stylers[".xml"] = std::make_unique<XmlStyler>();
    m_stylers[".lsx"] = m_stylers[".xml"];
    m_stylers[".xaml"] = m_stylers[".xml"];
    m_stylers[".xslt"] = m_stylers[".xml"];
    m_default = std::make_unique<PlainTextStyler>();
}

DocStyler::Ptr DocStylerRegistry::GetStyler(const CString& path)
{
    auto extension = ATLPath::FindExtension(path);

    auto utf8Extension = StringHelper::toUTF8(extension);

    const auto& instance = GetInstance();

    auto it = instance.m_stylers.find(utf8Extension.GetString());
    if (it != instance.m_stylers.end()) {
        return it->second;
    }

    return instance.m_default;
}

DocStyler::Ptr DocStylerRegistry::GetDefaultStyler()
{
    return GetInstance().m_default;
}
