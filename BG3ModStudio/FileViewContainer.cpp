#include "stdafx.h"
#include "FileViewContainer.h"
#include "FileViewFactory.h"

BOOL FileViewContainer::LoadView(const CString& path, const ByteBuffer& contents)
{
    CRect rc;
    GetClientRect(&rc);

    if (m_pFileView && *m_pFileView) {
        m_pFileView->Destroy();
    }

    m_pFileView = FileViewFactory::CreateFileView(path, contents, *this, rcDefault);
    if (!m_pFileView) {
        return FALSE;
    }

    ::SetWindowPos(*m_pFileView, nullptr, 0, 0, rc.Width(), rc.Height(),
                   SWP_NOZORDER | SWP_NOACTIVATE);

    Invalidate();

    return true;
}

LRESULT FileViewContainer::OnCreate(LPCREATESTRUCT pcs)
{
    return 0;
}

void FileViewContainer::OnSize(UINT nType, CSize size)
{
    if (m_pFileView && *m_pFileView) {
        ::SetWindowPos(*m_pFileView, nullptr, 0, 0, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void FileViewContainer::OnDestroy()
{
    if (m_pFileView && *m_pFileView) {
        m_pFileView->Destroy();
    }

    m_pFileView.reset();
}
