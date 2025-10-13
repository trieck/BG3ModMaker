#pragma once
#include "IFileView.h"

class FileViewContainer : public CWindowImpl<FileViewContainer>
{
    using Base = CWindowImpl;

    BEGIN_MSG_MAP(FileViewContainer)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

    DECLARE_WND_CLASS_EX(_T("FileViewContainer"), 0, COLOR_APPWORKSPACE)

    BOOL LoadView(const CString& path, const ByteBuffer& contents, FileViewFlags flags = FileViewFlags::None);

private:
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void OnSize(UINT nType, CSize size);
    void OnDestroy();

    IFileView::Ptr m_pFileView;
};
