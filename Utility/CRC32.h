#pragma once

class CRC32
{
public:
    static uint32_t compute(const uint8_t* data, size_t length);
};
