#pragma once

#include <format>

class Exception : public std::exception
{
public:
    explicit Exception(DWORD errCode);
    explicit Exception(const std::string& message);

    template <typename... Args>
    explicit Exception(std::format_string<Args...> fmt, Args&&... args);

    char const* what() const noexcept override;
    DWORD code() const noexcept;

private:
    std::string m_message;
    DWORD m_errorCode = ERROR_SUCCESS;
};

template<typename ...Args>
Exception::Exception(std::format_string<Args...> fmt, Args && ...args)
{
    m_message = std::format(fmt, std::forward<Args>(args)...);
}