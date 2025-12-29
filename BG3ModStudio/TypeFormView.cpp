#include "stdafx.h"
#include "OsiData.h"
#include "TypeFormView.h"

#include "StringHelper.h"
#include "Util.h"

HWND TypeFormView::Create(HWND hWndParent, LPARAM dwInitParam)
{
    if (!Base::Create(hWndParent, dwInitParam)) {
        return nullptr;
    }

    auto* pData = reinterpret_cast<OsiData*>(dwInitParam);
    ATLASSERT(pData != nullptr);

    m_pStory = pData->pStory;
    ATLASSERT(m_pStory != nullptr);

    m_pType = static_cast<const OsiType*>(pData->pdata);
    ATLASSERT(m_pType != nullptr);

    m_font.Attach(Util::CreateDialogFont(100, TRUE));

    auto wndType = GetDlgItem(IDC_TYPE);
    ATLASSERT(wndType.IsWindow());
    wndType.SetWindowText(StringHelper::fromUTF8(m_pType->name.c_str()));
    wndType.SetFont(m_font);

    auto wndAlias = GetDlgItem(IDC_ALIAS);
    ATLASSERT(wndAlias.IsWindow());
    wndAlias.SetFont(m_font);

    if (m_pType->alias != 0) {
        auto type = m_pStory->typeName(m_pType->alias);
        wndAlias.SetWindowText(StringHelper::fromUTF8(type.c_str()));
    } else {
        wndAlias.SetWindowText(L"(none)");
    }

    DlgResize_Init();

    return this->m_hWnd;
}

void TypeFormView::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);
}
