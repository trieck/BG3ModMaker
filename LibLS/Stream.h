#pragma once

#include "Exception.h"
#include "framework.h"

class Stream
{
public:
    Stream();
    Stream(const char* buf, size_t size);
    explicit Stream(const ByteBuffer& buf);
    explicit Stream(const std::string& str);
    virtual ~Stream();

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    Stream(Stream&&) = delete;

    template <typename T>
    T read();

    void read(char* buf, size_t size) const;

    void seek(int64_t offset, SeekMode mode) const;
    size_t tell() const;
    size_t size() const;

    using Ptr = std::unique_ptr<Stream>;
    static Ptr makeStream(const char* buf, size_t size);
    static Ptr makeStream(const ByteBuffer& buf);
    static Ptr makeStream(const std::string& str);

    Ptr read(uint32_t bytes) const;

    std::string str() const;
    std::istream& stream() const;

private:
    IStreamPtr m_stream;
};

template <typename T>
T Stream::read()
{
    T value;
    m_stream->read(reinterpret_cast<char*>(&value), sizeof(T));
    if (m_stream->fail()) {
        throw Exception(std::format("Failed to read {} bytes from stream", sizeof(T)));
    }

    return value;
}

