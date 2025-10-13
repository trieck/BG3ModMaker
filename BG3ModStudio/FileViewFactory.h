#pragma once

#include "IFileView.h"

class FileViewFactory
{
public:


    static IFileView::Ptr CreateFileView(const CString& path, HWND parent = nullptr, _U_RECT rect = nullptr,
                                         DWORD dwStyle = 0, DWORD dwStyleEx = 0, FileViewFlags = FileViewFlags::None);

    static IFileView::Ptr CreateFileView(const CString& path, const ByteBuffer& contents,
                                         HWND hWndParent = nullptr, _U_RECT rect = nullptr, DWORD dwStyle = 0,
                                         DWORD dwStyleEx = 0, FileViewFlags = FileViewFlags::None);

    static IFileView::Ptr CreateFileView(HWND parent = nullptr, _U_RECT rect = nullptr, DWORD dwStyle = 0,
                                         DWORD dwStyleEx = 0, FileViewFlags = FileViewFlags::None);
};
