#include "stdafx.h"
#include "FileView.h"

LRESULT FileView::OnCreate(LPCREATESTRUCT pcs)
{
    auto lRet = DefWindowProc();

    SetModify(FALSE);

    return lRet;
}

LRESULT FileView::OnSelChanged(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}
