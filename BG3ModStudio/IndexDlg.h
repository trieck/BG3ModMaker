#pragma once

#include "Indexer.h"
#include "ModelessDialog.h"
#include "resources/resource.h"

class IndexDlg : public ModelessDialog<IndexDlg>, public IIndexProgressListener
{
public:
    enum { IDD = IDD_INDEX };

    BEGIN_MSG_MAP(IndexDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnCancelRequested)
        COMMAND_ID_HANDLER3(IDC_B_PAKFILE, OnPakFile)
        COMMAND_ID_HANDLER3(IDC_B_INDEXPATH, OnIndexPath)
        COMMAND_ID_HANDLER3(IDOK, OnIndex)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancelRequested)
    END_MSG_MAP()

    // IIndexProgressListener
    void onStart(std::size_t totalEntries) override;
    void onFinished(std::size_t entries) override;
    void onFileIndexing(std::size_t index, const std::string& filename) override;
    bool isCancelled() override;
    void onCancel() override;

private:
    enum State
    {
        IDLE,
        INDEXING,
        CANCELING,
        CANCELLED
    };

    State m_state = IDLE;
    CEdit m_pakFile;
    CEdit m_indexPath;
    CButton m_indexButton;
    CButton m_overwriteCheckbox;
    CProgressBarCtrl m_progress;

    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    void OnCancelRequested();
    void OnIndex();
    void OnIndexPath();
    void OnPakFile();

    void Index(const CString& pakFile, const CString& indexPath);
};
