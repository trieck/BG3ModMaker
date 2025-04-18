#pragma once

struct UUIDT
{
    UUIDT() = default;
    UUIDT(const UUIDT&) = default;
    UUIDT(UUIDT&&) = default;
    UUIDT& operator=(const UUIDT&) = default;
    UUIDT& operator=(UUIDT&&) = default;
    ~UUIDT() = default;

    explicit UUIDT(const uint8_t* bytes);
    explicit UUIDT(const std::string& str);

    static UUIDT fromString(const std::string& input);
    std::string str() const;

    bool operator==(const UUIDT& other) const;
    bool operator!=(const UUIDT& other) const;

    std::array<uint8_t, 16> m_bytes{};
};

