#include "stdafx.h"
#include "FileStream.h"
#include "GoalFormView.h"
#include "OsiData.h"
#include "StringHelper.h"
#include "Util.h"

HWND GoalFormView::Create(HWND hWndParent, LPARAM dwInitParam)
{
    if (!Base::Create(hWndParent, dwInitParam)) {
        return nullptr;
    }

    auto* pData = reinterpret_cast<OsiData*>(dwInitParam);
    ATLASSERT(pData != nullptr);

    m_pStory = pData->pStory;
    ATLASSERT(m_pStory != nullptr);

    m_pGoal = static_cast<const OsiGoal*>(pData->pdata);
    ATLASSERT(m_pGoal != nullptr);

    m_decompiled = GetDlgItem(IDC_DECOMPILED);
    ATLASSERT(m_decompiled.IsWindow());

    m_font.Attach(Util::CreateFixedWidthFont(110));
    m_decompiled.SetFont(m_font);

    DlgResize_Init();

    Decompile();

    return this->m_hWnd;
}

void GoalFormView::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);
}

void GoalFormView::Decompile()
{
    Stream stream;
    m_pGoal->decompile(*m_pStory, stream);
    auto decompiled = StringHelper::fromUTF8(stream.str().c_str());
    decompiled.Replace(L"\n", L"\r\n");

    m_decompiled.SetWindowText(decompiled);
}

