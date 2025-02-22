#pragma once

#include "Exception.h"
#include "framework.h"
#include "IStreamBase.h"

class Stream : public IStreamBase
{
public:
    Stream();
    Stream(const char* buf, size_t size);
    explicit Stream(const ByteBuffer& buf);
    explicit Stream(const std::string& str);
    ~Stream() override;

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    Stream(Stream&&) = delete;

    template <typename T>
    T read();

    // IStreamBase
    size_t read(char* buf, size_t size) const override;
    size_t write(const char* buf, size_t size) const override;
    void seek(int64_t offset, SeekMode mode) const override;
    size_t tell() const override;
    size_t size() const override;

    using Ptr = std::unique_ptr<Stream>;
    static Ptr makeStream(const char* buf, size_t size);
    static Ptr makeStream(const ByteBuffer& buf);
    static Ptr makeStream(const std::string& str);

    Ptr read(size_t bytes) const;
    std::string str() const;
    ByteBuffer bytes() const;

private:
    IOStreamPtr m_stream;
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

