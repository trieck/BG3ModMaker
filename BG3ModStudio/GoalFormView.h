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
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(GoalFormView)
        DLGRESIZE_CONTROL(IDC_DECOMPILED, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    // IFormView
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override;

private:
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void Decompile();

    const OsiStory* m_pStory = nullptr;
    const OsiGoal* m_pGoal = nullptr;
    CEdit m_decompiled;
    CFont m_font;
};
