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

    ctrl.SetForeStyle(SCE_H_TAG, RGB(0, 38, 255));
    ctrl.SetForeStyle(SCE_H_ATTRIBUTE, RGB(220, 0, 100));
    ctrl.SetForeStyle(SCE_H_DOUBLESTRING, RGB(127, 0, 127));
    ctrl.SetForeStyle(SCE_H_COMMENT, RGB(0, 128, 0));
}

void JsonStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetLexer("json");
    ctrl.SetIndent(2);
    ctrl.SetTabWidth(2);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);

    ctrl.SetForeStyle(SCE_JSON_PROPERTYNAME, RGB(127, 0, 85));
    ctrl.SetForeStyle(SCE_JSON_STRING, RGB(42, 150, 0));
    ctrl.SetForeStyle(SCE_JSON_NUMBER, RGB(0, 0, 255));
    ctrl.SetForeStyle(SCE_JSON_OPERATOR, RGB(0, 0, 0));
}

void PlainTextStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetLexer(nullptr);
    ctrl.SetIndent(2);
    ctrl.SetTabWidth(2);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);
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
