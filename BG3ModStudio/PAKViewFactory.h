#pragma once
#include "IFileView.h"
#include "UtilityBase.h"

class PAKViewFactory
{
public:
    static IFileView::Ptr CreateFileView(const CString& path, const ByteBuffer& contents,
        HWND hWndParent = nullptr, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0);
};
