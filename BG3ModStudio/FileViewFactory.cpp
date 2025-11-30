#include "stdafx.h"
#include "BinaryFileView.h"
#include "Exception.h"
#include "FileStream.h"
#include "FileViewFactory.h"
#include "GR2FileView.h"
#include "GR2Reader.h"
#include "ImageView.h"
#include "LocaFileView.h"
#include "LSFFileView.h"
#include "TextFileView.h"

namespace { // anonymous namespace

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

BOOL IsBinaryFile(FileStream& stream)
{
    stream.seek(0, SeekMode::Begin);

    char buffer[1024];
    auto read = stream.read(buffer, sizeof(buffer));

    for (auto i = 0u; i < read; ++i) {
        if (buffer[i] == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsLocaFile(const ByteBuffer& contents)
{
    if (contents.second < 4) {
        return FALSE;
    }

    return (memcmp(contents.first.get(), "LOCA", 4) == 0);
}

BOOL IsLocaFile(FileStream& stream)
{
    stream.seek(0, SeekMode::Begin);

    char header[4];
    auto read = stream.read(header, sizeof(header));
    if (read < sizeof(header)) {
        return FALSE;
    }

    return (memcmp(header, "LOCA", 4) == 0);
}

BOOL IsLSFFile(const ByteBuffer& contents)
{
    if (contents.second < 4) {
        return FALSE;
    }

    return (memcmp(contents.first.get(), "LSOF", 4) == 0);
}

BOOL IsLSFFile(FileStream& stream)
{
    stream.seek(0, SeekMode::Begin);

    char header[4];
    auto read = stream.read(header, sizeof(header));
    if (read < sizeof(header)) {
        return FALSE;
    }

    return (memcmp(header, "LSOF", 4) == 0);
}

BOOL IsGR2File(FileStream& stream)
{
    stream.seek(0, SeekMode::Begin);

    return GR2Reader::isGR2(stream);
}

BOOL IsGR2File(const ByteBuffer& contents)
{
    return GR2Reader::isGR2(contents);
}
} // anonymous namespace

IFileView::Ptr FileViewFactory::CreateFileView(const CString& path, HWND parent, _U_RECT rect, DWORD dwStyle,
                                               DWORD dwStyleEx, FileViewFlags flags)
{
    IFileView::Ptr fileView;

    CStringA strPath(path);
    FileStream stream;

    try {
        stream.open(strPath, "rb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return nullptr;
    }

    if (ImageView::IsRenderable(path)) {
        fileView = std::make_shared<ImageView>();
    } else if (IsLocaFile(stream)) {
        fileView = std::make_shared<LocaFileView>();
    } else if (IsLSFFile(stream)) {
        fileView = std::make_shared<LSFFileView>();
    } else if (IsGR2File(stream)) {
        fileView = std::make_shared<GR2FileView>();
    } else if (IsBinaryFile(stream)) {
        fileView = std::make_shared<BinaryFileView>();
    } else {
        auto textView = std::make_shared<TextFileView>();
        if (textView != nullptr && flags == FileViewFlags::ReadOnly) {
            textView->SetReadOnly(TRUE);
        }
        fileView = textView;
    }

    if (fileView != nullptr) {
        if (!fileView->Create(parent, rect, dwStyle, dwStyleEx)) {
            ATLTRACE("Failed to create file view.\n");
            return nullptr;
        }
    }

    return fileView;
}

IFileView::Ptr FileViewFactory::CreateFileView(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx,
                                               FileViewFlags flags)
{
    IFileView::Ptr fileView;

    auto textView = std::make_shared<TextFileView>();
    if (textView == nullptr) {
        ATLTRACE("Failed to create file view.\n");
        return nullptr;
    }

    if (flags == FileViewFlags::ReadOnly) {
        textView->SetReadOnly(TRUE);
    }

    if (!textView->Create(parent, rect, dwStyle, dwStyleEx)) {
        ATLTRACE("Failed to create file view window.\n");
        return nullptr;
    }

    fileView = textView;

    return fileView;
}

IFileView::Ptr FileViewFactory::CreateFileView(const CString& path, const ByteBuffer& contents,
                                               HWND hWndParent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx,
                                               FileViewFlags flags)
{
    IFileView::Ptr fileView;

    if (ImageView::IsRenderable(path)) {
        fileView = std::make_shared<ImageView>();
    } else if (IsLocaFile(contents)) {
        fileView = std::make_shared<LocaFileView>();
    } else if (IsLSFFile(contents)) {
        fileView = std::make_shared<LSFFileView>();
    } else if (IsGR2File(contents)) {
        fileView = std::make_shared<GR2FileView>();
    } else if (IsBinaryFile(contents)) {
        fileView = std::make_shared<BinaryFileView>();
    } else {
        auto textView = std::make_shared<TextFileView>();
        if (textView != nullptr && flags == FileViewFlags::ReadOnly) {
            textView->SetReadOnly(TRUE);
        }
        fileView = textView;
    }

    if (fileView != nullptr) {
        if (!fileView->Create(hWndParent, rect, dwStyle, dwStyleEx)) {
            ATLTRACE("Failed to create file view.\n");
            return nullptr;
        }
    }

    if (!fileView->LoadBuffer(path, contents)) {
        fileView->Destroy();
        ATLTRACE("Failed to load buffer into view.\n");
        return nullptr;
    }

    return fileView;
}
