#pragma once

#include "FormView.h"
#include "OsiData.h"
#include "OsiStory.h"
#include "resources/resource.h"

class FunctionFormView : public DialogFormImpl<FunctionFormView>,
                         public CDialogResize<FunctionFormView>
{
public:
    using Base = DialogFormImpl;

    enum { IDD = IDD_FUNCTION_VIEW };

    BEGIN_MSG_MAP(FunctionFormView)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(FunctionFormView)
        DLGRESIZE_CONTROL(IDC_E_SIGNATURE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    // IFormView
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override;

private:
    void OnSize(UINT /*uMsg*/, const CSize& size);

    CString GetSignature() const;

    const OsiStory* m_pStory = nullptr;
    const OsiFunction* m_pFunction = nullptr;

    CEdit m_signature;
    CFont m_font, m_boldFont;
};
