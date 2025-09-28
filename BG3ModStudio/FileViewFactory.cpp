#include "stdafx.h"
#include "BinaryFileView.h"
#include "ImageView.h"
#include "Exception.h"
#include "FileStream.h"
#include "FileViewFactory.h"
#include "TextFileView.h"

namespace { // anonymous namespace

BOOL IsDDSFile(const CString& path)
{
    CStringA strPath(path);
    FileStream file;

    try {
        file.open(strPath, "rb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

    char magic[4];
    auto read = file.read(magic, sizeof(magic));
    if (read != sizeof(magic)) {
        return FALSE;
    }

    return (magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ');
}

BOOL IsBinaryFile(const CString& path)
{
    CStringA strPath(path);

    FileStream file;
    try {
        file.open(strPath, "rb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

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

IFileView::Ptr FileViewFactory::CreateFileView(const CString& path, HWND parent, _U_RECT rect, DWORD dwStyle,
                                               DWORD dwStyleEx)
{
    IFileView::Ptr fileView;

    if (IsDDSFile(path)) {
        fileView = std::make_shared<ImageView>();
    } else if (IsBinaryFile(path)) {
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

IFileView::Ptr FileViewFactory::CreateFileView(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    IFileView::Ptr fileView;

    fileView = std::make_shared<TextFileView>();
    if (fileView == nullptr) {
        ATLTRACE("Failed to create file view.\n");
        return nullptr;
    }

    if (!fileView->Create(parent, rect, dwStyle, dwStyleEx)) {
        ATLTRACE("Failed to create file view window.\n");
        return nullptr;
    }

    return fileView;
}
