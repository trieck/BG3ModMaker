#include "stdafx.h"
#include "PAKViewFactory.h"

#include "BinaryFileView.h"
#include "ImageView.h"
#include "LSFFileView.h"
#include "TextFileView.h"

namespace { // anonymous namespace

BOOL IsLSFFile(const ByteBuffer& contents)
{
    if (contents.second < 4) {
        return FALSE;
    }

    return (memcmp(contents.first.get(), "LSOF", 4) == 0);
}

BOOL IsBinaryFile(const ByteBuffer& contents)
{
    auto read = std::min(contents.second, static_cast<size_t>(1024));

    for (auto i = 0u; i < read; ++i) {
        if (contents.first[i] == 0) {
            return TRUE;
        }
    }

    return FALSE;
}
} // anonymous namespace

IFileView::Ptr PAKViewFactory::CreateFileView(const CString& path, const ByteBuffer& contents,
                                              HWND hWndParent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    IFileView::Ptr fileView;

    if (ImageView::IsRenderable(path)) {
        fileView = std::make_shared<ImageView>();
    } else if (IsLSFFile(contents)) {
        fileView = std::make_shared<LSFFileView>();
    } else if (IsBinaryFile(contents)) {
        fileView = std::make_shared<BinaryFileView>();
    } else {
        fileView = std::make_shared<TextFileView>();
    }

    if (fileView != nullptr) {
        if (!fileView->Create(hWndParent, rect, dwStyle, dwStyleEx)) {
            ATLTRACE("Failed to create file view.\n");
            return nullptr;
        }
    }

    if (!fileView->LoadBuffer(contents)) {
        fileView->Destroy();
        ATLTRACE("Failed to load buffer into view.\n");
        return nullptr;
    }

    return fileView;
}
