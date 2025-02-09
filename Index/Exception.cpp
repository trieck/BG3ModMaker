#include "stdafx.h"
#include "Exception.h"

Exception::Exception(DWORD errCode): m_errorCode(errCode)
{
    LPVOID errMsg;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errCode, 0, reinterpret_cast<LPSTR>(&errMsg), 0, nullptr);

    m_message = "Error (Code " + std::to_string(errCode) + "): " + (errMsg
                                                                        ? static_cast<LPSTR>(errMsg)
                                                                        : "Unknown error");

    if (errMsg) {
        LocalFree(errMsg);
    }
}

const char* Exception::what() const noexcept
{
    return m_message.c_str();
}

DWORD Exception::code() const noexcept
{
    return m_errorCode;
}
