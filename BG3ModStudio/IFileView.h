#pragma once

enum FileEncoding {
    UNKNOWN = -1,
    ANSI,
    UTF8,
    UTF8BOM,
    UTF16LE,
    UTF16BE,
    UTF32LE,
    UTF32BE
};

class IFileView
{
public:
    virtual ~IFileView() = default;
    using Ptr = std::shared_ptr<IFileView>;

    virtual BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) = 0;
    virtual BOOL LoadFile(const CString& path) = 0;
    virtual BOOL SaveFile(const CString& path) = 0;
    virtual BOOL Destroy() = 0;
    virtual LPCTSTR GetPath() const = 0;
    virtual FileEncoding GetEncoding() const = 0;
    virtual operator HWND() const = 0;
};
