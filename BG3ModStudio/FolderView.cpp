#include "stdafx.h"
#include "FolderView.h"

LRESULT FolderView::OnCreate(LPCREATESTRUCT)
{
    auto bResult = DefWindowProc();


    SetMsgHandled(FALSE);

    return bResult;
}

void FolderView::OnDestroy()
{
}

LRESULT FolderView::OnItemExpanding(LPNMHDR pnmh)
{
    return 0;
}

LRESULT FolderView::OnDelete(LPNMHDR pnmh)
{
    return 0;
}
