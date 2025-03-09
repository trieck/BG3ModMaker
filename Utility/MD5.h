#pragma once

class MD5
{
public:
    MD5();
    ~MD5();

    void update(const uint8_t* buf, uint32_t size);
    void finalize(uint8_t digest[16]);
    std::string digestString(const std::string& str);
private:
    void init();

    DECLARE_HANDLE(HMD5);
    HMD5 m_hMD5{};
};
