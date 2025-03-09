#pragma once

class Exception : public std::exception {

public:
    explicit Exception(DWORD errCode);
    explicit Exception(const std::string& message);
        
    char const* what() const noexcept override;
    DWORD code() const noexcept;

private:
    std::string m_message;
    DWORD m_errorCode;
};
