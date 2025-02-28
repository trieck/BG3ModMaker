#pragma once

class FileOperation
{
public:
    FileOperation() = default;
    ~FileOperation();

    HRESULT Create(DWORD dwOperationFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOFX_RECYCLEONDELETE);
    HRESULT DeleteItem(const CString& path);

    void Release();

private:
    CComPtr<IFileOperation> fileOperation;
};

