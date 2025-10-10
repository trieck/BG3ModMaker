#pragma once

#include <atlstr.h>
#include <atlwin.h>

class COMError
{
public:
    explicit COMError(HRESULT hr, LPUNKNOWN pUnknown = nullptr, REFIID iid = GUID_NULL);
    operator CString() const;

private:
    void SetErrorMessage(HRESULT hr, LPUNKNOWN pUnknown, REFIID iid);

    CString m_message;
};

#define COM_ERROR_DESC(hr, pUnk) \
    ((CString)COMError((hr), (pUnk), __uuidof((pUnk))))

int CoMessageBox(HWND hWndOwner,
    HRESULT hr,
    LPUNKNOWN pUnknown = nullptr,
    _U_STRINGorID title = static_cast<LPCTSTR>(nullptr),
    UINT uType = MB_OK | MB_ICONINFORMATION);
