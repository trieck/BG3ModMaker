#pragma once

#include "FormView.h"
#include "OsiStory.h"
#include "resources/resource.h"

class TypeFormView : public DialogFormImpl<TypeFormView>,
                     public CDialogResize<TypeFormView>
{
public:
    using Base = DialogFormImpl;

    enum { IDD = IDD_TYPE_VIEW };

    BEGIN_MSG_MAP(TypeFormView)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(TypeFormView)

    END_DLGRESIZE_MAP()

    // IFormView
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override;

private:
    void OnSize(UINT /*uMsg*/, const CSize& size);

    const OsiStory* m_pStory = nullptr;
    const OsiType* m_pType = nullptr;
    CFont m_font;
};
