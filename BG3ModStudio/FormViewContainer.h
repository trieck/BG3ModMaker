#pragma once
#include "FormView.h"
#include "OsiViewFactory.h"

class FormViewContainer : public CWindowImpl<FormViewContainer>
{
    using Base = CWindowImpl;

    BEGIN_MSG_MAP(FormViewContainer)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

    DECLARE_WND_CLASS_EX(_T("FormViewContainer"), 0, COLOR_APPWORKSPACE)

    BOOL LoadView(OsiViewType viewType, LPCVOID pv = nullptr);
    void DestroyView();

private:
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    void UpdateLayout();

    IFormView::Ptr m_pFormView;
};
