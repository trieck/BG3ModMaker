#pragma once

struct FormatToken
{
    LONG start{};
    LONG end{};
    uint32_t reserved{};
    COLORREF color{};
};

using FormatTokenVec = std::vector<FormatToken>;
