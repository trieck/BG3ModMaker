#pragma once

#include "StreamBase.h"
#include "UtilityBase.h"

class Stream : public StreamBase
{
public:
    using StreamBase::read;
    using StreamBase::write;

    Stream();
    explicit Stream(size_t capacity);
    Stream(const char* buf, size_t size);
    explicit Stream(const ByteBuffer& buf);
    explicit Stream(const std::string& str);
    ~Stream() override;

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    Stream(Stream&&) noexcept;
    Stream& operator=(Stream&&) noexcept;

    // StreamBase
    size_t read(char* buf, size_t size) override;
    size_t write(const char* buf, size_t size) override;
    void seek(int64_t offset, SeekMode mode) override;
    size_t tell() const override;
    size_t size() const override;

    static Stream makeStream(UInt8Ptr&& ptr, size_t size);
    static Stream makeStream(const char* buf, size_t size);
    static Stream makeStream(const ByteBuffer& buf);
    static Stream makeStream(const std::string& str);
    static Stream makeStream(StreamBase& stream);

    Stream read(size_t bytes);
    std::string str() const;
    ByteBuffer bytes() const;
    size_t capacity() const;

    void attach(ByteBuffer buffer);
    ByteBuffer detach();

private:
    void alloc(size_t size);
    void realloc(size_t size);

    std::size_t m_pos, m_size, m_capacity;
    UInt8Ptr m_bytes;
};
