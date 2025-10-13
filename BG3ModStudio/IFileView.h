#pragma once
#include "UtilityBase.h"

enum FileEncoding
{
    UNKNOWN = -1,
    ANSI,
    UTF8,
    UTF8BOM,
    UTF16LE,
    UTF16BE,
    UTF32LE,
    UTF32BE
};

enum class FileViewFlags : uint32_t
{
    None = 0,
    ReadOnly = 1 << 0,
};

class IFileView
{
public:
    virtual ~IFileView() = default;
    using Ptr = std::shared_ptr<IFileView>;

    virtual BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) = 0;
    virtual BOOL LoadFile(const CString& path) = 0;
    virtual BOOL LoadBuffer(const CString& path, const ByteBuffer& buffer) = 0;
    virtual BOOL SaveFile() = 0;
    virtual BOOL SaveFileAs(const CString& path) = 0;
    virtual BOOL Destroy() = 0;
    virtual BOOL IsDirty() const = 0;
    virtual const CString& GetPath() const = 0;
    virtual VOID SetPath(const CString& path) = 0;
    virtual FileEncoding GetEncoding() const = 0;
    virtual operator HWND() const = 0;
};
