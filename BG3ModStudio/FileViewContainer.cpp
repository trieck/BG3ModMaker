#include "stdafx.h"
#include "FileViewContainer.h"
#include "PAKViewFactory.h"

BOOL FileViewContainer::LoadView(const CString& path, const ByteBuffer& contents)
{
    CRect rc;
    GetClientRect(&rc);

    if (m_pFileView && *m_pFileView) {
        m_pFileView->Destroy();
    }

    m_pFileView = PAKViewFactory::CreateFileView(path, contents, *this, rc);
    if (!m_pFileView) {
        return FALSE;
    }

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
