#pragma once

class FileOperation
{
public:
    FileOperation() = default;
    ~FileOperation();

    HRESULT Create(DWORD dwOperationFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOFX_RECYCLEONDELETE);
    HRESULT DeleteItem(const CString& path);
    HRESULT RenameItems(const CString& oldPath, const CString& newPath);

    void Release();

private:
    CComPtr<IFileOperation> fileOperation;
};
