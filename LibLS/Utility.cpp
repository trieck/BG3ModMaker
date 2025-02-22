#include "pch.h"
#include "Utility.h"

std::string comma(int64_t i)
{
    std::ostringstream oss;
    oss.imbue(std::locale(""));
    oss << std::fixed << i;
    return oss.str();
}

IStreamPtr createMemoryStream(const uint8Ptr& data, size_t size)
{
    return std::make_unique<std::stringstream>(
        std::string(reinterpret_cast<char*>(data.get()), size),
        std::ios::binary);
}

IStreamPtr createMemoryStream(const std::string& data)
{
    return std::make_unique<std::stringstream>(data, std::ios::binary);
}

IStreamPtr createMemoryStream(const ByteBuffer& data)
{
    return std::make_unique<std::stringstream>(
        std::string(reinterpret_cast<const char*>(data.first.get()), data.second),
        std::ios::binary);
}

IStreamPtr createMemoryStream(const char* data, size_t size)
{
    return std::make_unique<std::stringstream>(
        std::string(data, size),
        std::ios::binary);
}

