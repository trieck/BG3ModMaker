#include "pch.h"
#include "Utility.h"

std::string comma(int64_t i)
{
    std::ostringstream oss;
    oss.imbue(std::locale(""));
    oss << std::fixed << i;
    return oss.str();
}

size_t closestFibonacci(size_t n)
{
    size_t a = 0;
    size_t b = 1;

    while (b < n) {
        size_t temp = b;
        b = a + b;
        a = temp;
    }

    return b;
}
