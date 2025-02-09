#pragma once

#include <exception>

class Exception : public std::exception {

public:
    explicit Exception(DWORD errCode);
        
    char const* what() const noexcept override;
    DWORD code() const noexcept;

private:
    std::string m_message;
    DWORD m_errorCode;
};
