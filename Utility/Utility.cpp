#include "pch.h"
#include "Utility.h"

std::string comma(int64_t i)
{
    std::ostringstream oss;
    oss.imbue(std::locale(""));
    oss << std::fixed << i;
    return oss.str();
}
