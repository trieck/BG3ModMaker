#include "pch.h"
#include "Exception.h"

Exception::Exception(DWORD errCode): m_errorCode(errCode)
{
    LPVOID errMsg;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errCode, 0, reinterpret_cast<LPSTR>(&errMsg), 0, nullptr);

    m_message = std::format("Error (Code {}): {}", errCode, errMsg ? static_cast<LPSTR>(errMsg) : "Unknown error");

    if (errMsg) {
        LocalFree(errMsg);
    }
}

Exception::Exception(const std::string& message) : m_errorCode(ERROR_SUCCESS)
{
    m_message = message;
}

const char* Exception::what() const noexcept
{
    return m_message.c_str();
}

DWORD Exception::code() const noexcept
{
    return m_errorCode;
}
