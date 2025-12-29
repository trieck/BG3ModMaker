#pragma once

#include "FormView.h"
#include "OsiStory.h"
#include "resources/resource.h"

class DatabaseFormView : public DialogFormImpl<DatabaseFormView>,
                         public CDialogResize<DatabaseFormView>
{
public:
    using Base = DialogFormImpl;

    enum { IDD = IDD_DB_VIEW };

    BEGIN_MSG_MAP(DatabaseFormView)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(DatabaseFormView)
        DLGRESIZE_CONTROL(IDC_LST_DATABASE, DLSZ_SIZE_X)
    END_DLGRESIZE_MAP()

    // IFormView
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override;

private:
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void OnTimer(UINT_PTR nIDEvent);

    void AutoAdjustColumns();
    void InsertColumns();
    void InsertFacts();

    const OsiStory* m_pStory = nullptr;
    const OsiDatabase* m_pDatabase = nullptr;
    const OsiNode* m_pOwnerNode = nullptr;

    CListViewCtrl m_view;
    CFont m_font;
};
