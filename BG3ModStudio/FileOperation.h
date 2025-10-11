#pragma once

class FileOperation
{
public:
    FileOperation() = default;
    ~FileOperation();

    HRESULT Create(DWORD dwOperationFlags =
        FOF_ALLOWUNDO | FOF_NOCONFIRMATION
        | FOF_NOCONFIRMMKDIR | FOFX_RECYCLEONDELETE);

    HRESULT NewFile(const CString& path);
    HRESULT NewFolder(const CString& path);
    HRESULT DeleteItem(const CString& path);
    HRESULT RenameItems(const CString& oldPath, const CString& newPath);

    void Release();

private:
    CComPtr<IFileOperation> fileOperation;
};
