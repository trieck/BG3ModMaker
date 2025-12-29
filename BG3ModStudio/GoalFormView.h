#pragma once

#include "FormView.h"
#include "OsiStory.h"
#include "resources/resource.h"

class GoalFormView : public DialogFormImpl<GoalFormView>,
                     public CDialogResize<GoalFormView>
{
public:
    using Base = DialogFormImpl;

    enum { IDD = IDD_GOAL_VIEW };

    BEGIN_MSG_MAP(GoalFormView)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnSelChanged)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(GoalFormView)
        DLGRESIZE_CONTROL(IDC_E_SIGNATURE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    // IFormView
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override;

private:
    LRESULT OnSelChanged(LPNMHDR pnmh);
    void OnSize(UINT /*uMsg*/, const CSize& size);

    CString CallString(const OsiCall& call);
    CString ParameterString(const OsiCall& call, uint32_t index, const OsiTypedValue::Ptr& param);

    void Populate();
    const OsiStory* m_pStory = nullptr;
    const OsiGoal* m_pGoal = nullptr;
    CTreeViewCtrlEx m_initCalls, m_exitCalls;
    CEdit m_signature;
    CImageList m_imageList;
    CFont m_font;
};
