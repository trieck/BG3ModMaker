#include "stdafx.h"
#include "FileViewFactory.h"

#include "BinaryFileView.h"
#include "FileStream.h"
#include "TextFileView.h"

namespace { // anonymous namespace
    BOOL IsBinaryFile(const CString& path)
    {
        CStringA strPath(path);

        FileStream file;
        file.open(strPath, "rb");

        char buffer[1024];
        auto read = file.read(buffer, sizeof(buffer));
        
        for (auto i = 0u; i < read; ++i) {
            if (buffer[i] == 0) {
                return TRUE;
            }
        }

        return FALSE;
    }

} // anonymous namespace

IFileView::Ptr FileViewFactory::CreateFileView(const CString& path, HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    IFileView::Ptr fileView;

    if (IsBinaryFile(path)) {
        fileView = std::make_shared<BinaryFileView>();
    } else {
        fileView = std::make_shared<TextFileView>();
    }

    if (fileView != nullptr) {
        if (!fileView->Create(parent, rect, dwStyle, dwStyleEx)) {
            ATLTRACE("Failed to create file view.\n");
            return nullptr;
        }
    }

    return fileView;
}
