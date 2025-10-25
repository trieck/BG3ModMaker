#include "stdafx.h"
#include "Win32ErrorMessage.h"

Win32ErrorMessage::Win32ErrorMessage(DWORD errorCode) : m_errorCode(errorCode)
{
}

CString Win32ErrorMessage::GetMessage() const
{
    LPVOID msgBuffer = nullptr;
    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        m_errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&msgBuffer),
        0,
        nullptr);

    CString message;
    if (size > 0 && msgBuffer != nullptr) {
        message = static_cast<LPCWSTR>(msgBuffer);
        LocalFree(msgBuffer);
    } else {
        message.Format(L"Unknown error code: %lu", m_errorCode);
    }

    return message;
}

CString Win32ErrorMessage::GetErrorMsg(DWORD errorCode)
{
    Win32ErrorMessage wem(errorCode);

    return wem.GetMessage();
}
