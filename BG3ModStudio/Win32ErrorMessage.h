#pragma once

class Win32ErrorMessage
{
public:
    explicit Win32ErrorMessage(DWORD errorCode = GetLastError());

    CString GetMessage() const;
    static CString GetErrorMsg(DWORD errorCode = GetLastError());

private:
    DWORD m_errorCode;
};
