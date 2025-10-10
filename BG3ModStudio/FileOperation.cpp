#include "stdafx.h"
#include "FileOperation.h"

FileOperation::~FileOperation()
{
    Release();
}

HRESULT FileOperation::Create(DWORD dwOperationFlags)
{
    if (fileOperation != nullptr) {
        fileOperation.Release();
    }

    auto hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOperation));
    if (FAILED(hr)) {
        ATLTRACE("Unable to create file operation instance.\n");
    }

    hr = fileOperation->SetOperationFlags(dwOperationFlags);
    if (FAILED(hr)) {
        ATLTRACE("Unable to set file operation flags.\n");
        return hr;
    }

    return hr;
}

void FileOperation::Release()
{
    if (fileOperation != nullptr) {
        fileOperation.Release();
    }
}

HRESULT FileOperation::DeleteItem(const CString& path)
{
    if (fileOperation == nullptr) {
        return E_FAIL;
    }

    CComPtr<IShellItem> pItem;
    auto hr = SHCreateItemFromParsingName(path, nullptr, IID_PPV_ARGS(&pItem));
    if (FAILED(hr)) {
        return hr;
    }

    hr = fileOperation->DeleteItem(pItem, nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return fileOperation->PerformOperations();
}

HRESULT FileOperation::RenameItems(const CString& oldPath, const CString& newPath)
{
    if (fileOperation == nullptr) {
        return E_FAIL;
    }

    CComPtr<IShellItem> pItem;
    auto hr = SHCreateItemFromParsingName(oldPath, nullptr, IID_PPV_ARGS(&pItem));
    if (FAILED(hr)) {
        return hr;
    }

    hr = fileOperation->RenameItem(pItem, newPath, nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return fileOperation->PerformOperations();
}
