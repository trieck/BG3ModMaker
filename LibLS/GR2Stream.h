#pragma once

#include "Stream.h"

class GR2Stream
{
public:
    GR2Stream() = default;
    explicit GR2Stream(uint8_t* buf);
    ~GR2Stream() = default;

    GR2Stream(const GR2Stream& rhs);
    GR2Stream& operator=(const GR2Stream&);

    template <typename T>
    T read() requires (StreamBinary<T>);

    uint8_t*& data();

private:
    uint8_t* m_buf{nullptr};
};

class GR2RefStream
{
public:
    GR2RefStream() = default;
    explicit GR2RefStream(uint8_t** buf);
    ~GR2RefStream() = default;
    explicit GR2RefStream(const GR2RefStream& rhs);
    GR2RefStream& operator=(const GR2RefStream&);

    template <typename T>
    T read() requires (StreamBinary<T>);

    bool isNull() const;

    uint8_t** data() const;

private:
    uint8_t** m_buf{nullptr};
};

template <typename T>
T GR2Stream::read() requires (StreamBinary<T>)
{
    T value;
    std::memcpy(static_cast<void*>(&value), m_buf, sizeof(T));

    m_buf += sizeof(T);

    return value;
}

template <typename T>
T GR2RefStream::read() requires (StreamBinary<T>)
{
    T value;
    std::memcpy(static_cast<void*>(&value), *m_buf, sizeof(T));

    *m_buf += sizeof(T);

    return value;
}
