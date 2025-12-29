#include "stdafx.h"
#include "FunctionFormView.h"
#include "StringHelper.h"
#include "Util.h"

HWND FunctionFormView::Create(HWND hWndParent, LPARAM dwInitParam)
{
    if (!Base::Create(hWndParent, dwInitParam)) {
        return nullptr;
    }

    auto* pData = reinterpret_cast<OsiData*>(dwInitParam);
    ATLASSERT(pData != nullptr);

    m_pStory = pData->pStory;
    ATLASSERT(m_pStory != nullptr);

    m_pFunction = static_cast<const OsiFunction*>(pData->pdata);
    ATLASSERT(m_pFunction != nullptr);

    auto functionType = GetDlgItem(IDC_FUNCTION_TYPE);
    ATLASSERT(functionType.IsWindow());
    functionType.SetWindowText(StringHelper::fromUTF8(m_pFunction->functionType().c_str()));

    m_signature = GetDlgItem(IDC_E_SIGNATURE);
    ATLASSERT(m_signature.IsWindow());

    m_font.Attach(Util::CreateFixedWidthFont(110));
    m_signature.SetFont(m_font);
    m_signature.SetWindowText(GetSignature());

    m_boldFont = Util::CreateDialogFont(100, TRUE);
    functionType.SetFont(m_boldFont);

    DlgResize_Init();

    return this->m_hWnd;
}

void FunctionFormView::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);
}

CString FunctionFormView::GetSignature() const
{
    CString signature;

    const auto& sig = m_pFunction->name;

    auto name = StringHelper::fromUTF8(sig.name.c_str());

    signature.Format(L"%s(", name.GetString());
    for (auto i = 0u; i < sig.parameters.types.size(); ++i) {
        const auto type = sig.parameters.types[i];

        if (sig.isOutParam(i)) {
            signature.Append(L"out ");
        }

        signature.Append(StringHelper::fromUTF8(m_pStory->typeName(type).c_str()));

        if (i < sig.parameters.types.size() - 1) {
            signature.Append(L",\r\n");
        }
    }

    signature.Append(L")");

    return signature;
}
