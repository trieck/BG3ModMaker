#include "stdafx.h"
#include "DocStyler.h"
#include "lexilla/include/SciLexer.h"

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
    m_stylers[".xml"] = std::make_unique<XmlStyler>();
    m_stylers[".lsx"] = m_stylers[".xml"];
    m_stylers[".json"] = std::make_unique<JsonStyler>();
    m_stylers[".lsj"] = m_stylers[".json"];
    m_stylers[".txt"] = std::make_unique<PlainTextStyler>();
    m_default = std::make_unique<PlainTextStyler>();
}

DocStyler::Ptr DocStylerRegistry::GetStyler(const CString& path) const
{
    auto extension = ATLPath::FindExtension(path);

    CW2AEX<MAX_PATH> ansiExtension(extension);

    auto it = m_stylers.find(static_cast<LPSTR>(ansiExtension));

    if (it != m_stylers.end()) {
        return it->second;
    }

    return m_default;
}

DocStyler::Ptr DocStylerRegistry::GetDefaultStyler() const
{
    return m_default;
}
