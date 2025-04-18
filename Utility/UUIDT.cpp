#include "pch.h"
#include "UUIDT.h"

static bool tryParseUUID(const std::string& input, UUIDT& out);
static bool tryParseExactD(std::string_view s, UUIDT& out);
static bool tryParseExactP(std::string_view s, UUIDT& out);
static bool tryParseExactB(std::string_view s, UUIDT& out);
static bool tryParseExactN(std::string_view s, UUIDT& out);
static bool tryParseExactX(std::string_view s, UUIDT& out);

UUIDT::UUIDT(const uint8_t* bytes)
{
    std::copy_n(bytes, 16, m_bytes.begin());
}

UUIDT::UUIDT(const std::string& str)
{
    if (!tryParseUUID(str, *this)) {
        throw std::invalid_argument("Invalid UUID format.");
    }
}

UUIDT UUIDT::fromString(const std::string& input)
{
    UUIDT out;

    if (!tryParseUUID(input, out)) {
        throw std::invalid_argument("Invalid UUID format.");
    }

    return out;
}

std::string UUIDT::str() const
{
    std::ostringstream result;

    for (size_t i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            result << '-';
        }
        result << std::format("{:02x}", m_bytes[i]);
    }

    return result.str();
}

bool UUIDT::operator==(const UUIDT& other) const
{
    return std::equal(std::begin(m_bytes), std::end(m_bytes), std::begin(other.m_bytes));
}

bool UUIDT::operator!=(const UUIDT& other) const
{
    return !(*this == other);
}

bool tryParseExactD(std::string_view s, UUIDT& out)
{
    if (s.size() != 36 || s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-') {
        return false;
    }

    auto hex = [](char c) -> int {
        if ('0' <= c && c <= '9') return c - '0';
        if ('a' <= c && c <= 'f') return 10 + (c - 'a');
        if ('A' <= c && c <= 'F') return 10 + (c - 'A');
        return -1;
        };

    auto parseHexByte = [&](int i) -> std::optional<uint8_t> {
        int hi = hex(s[i]);
        int lo = hex(s[i + 1]);
        if (hi == -1 || lo == -1) {
            return std::nullopt;
        }

        return static_cast<uint8_t>((hi << 4) | lo);
        };

    static constexpr int positions[] = {
        0, 2, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 28, 30, 32, 34
    };

    for (int i = 0; i < 16; ++i) {
        auto byte = parseHexByte(positions[i]);
        if (!byte) {
            return false;
        }

        out.m_bytes[i] = *byte;
    }

    return true;
}

bool tryParseExactP(std::string_view s, UUIDT& out)
{
    if (s.size() != 38 || s[0] != '(' || s[37] != ')') {
        return false;
    }

    return tryParseExactD(s.substr(1, 36), out);
}

bool tryParseExactB(std::string_view s, UUIDT& out)
{
    if (s.size() != 38 || s[0] != '{' || s[37] != '}') {
        return false;
    }

    return tryParseExactD(s.substr(1, 36), out);
}

bool tryParseExactN(std::string_view s, UUIDT& out)
{
    if (s.size() != 32) {
        return false;
    }

    auto hex = [](char c) -> int {
        if ('0' <= c && c <= '9') return c - '0';
        if ('a' <= c && c <= 'f') return 10 + (c - 'a');
        if ('A' <= c && c <= 'F') return 10 + (c - 'A');
        return -1;
        };

    for (size_t i = 0; i < 16; ++i) {
        auto hi = hex(s[i * 2]);
        auto lo = hex(s[i * 2 + 1]);

        if (hi == -1 || lo == -1) {
            return false;
        }

        out.m_bytes[i] = static_cast<uint8_t>((hi << 4) | lo);
    }

    return true;
}

bool tryParseExactX(std::string_view s, UUIDT& out)
{
    // e.g. "{0xd85b1407,0x351d,0x4694,{0x93,0x92,0x03,0xac,0xc5,0x87,0x0e,0xb1}}"

    // Compat notes due to the previous implementation's implementation details.
    // - Each component need not be the full expected number of digits.
    // - Each component may contain any number of leading 0s
    // - The "short" components are parsed as 32-bits and only considered to overflow if they'd overflow 32 bits.
    // - The "byte" components are parsed as 32-bits and are considered to overflow if they'd overflow 8 bits,
    //   but for the Guid ctor, whether they overflow 8 bits or 32 bits results in differing exceptions.
    // - Components may begin with "0x", "0x+", even "0x+0x".
    // - "0X" is valid instead of "0x"

    return false; // Not implemented
}

bool tryParseUUID(const std::string& input, UUIDT& out)
{
    std::string_view trimmed = input; // Implement `trim()` or use your helper

    if (trimmed.size() < 32) { // minimal length we can parse
        return false;
    }

    if (trimmed.front() == '(') {
        return tryParseExactP(trimmed, out);
    }

    if (trimmed.front() == '{') {
        return trimmed[9] == '-' ? tryParseExactB(trimmed, out) : tryParseExactX(trimmed, out);
    }

    if (trimmed[8] == '-') {
        return tryParseExactD(trimmed, out);
    }

    return tryParseExactN(trimmed, out);
}
