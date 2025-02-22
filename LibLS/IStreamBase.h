#pragma once

enum class SeekMode : DWORD {
    Begin = FILE_BEGIN,
    Current = FILE_CURRENT,
    End = FILE_END
};

class IStreamBase
{
public:
    virtual ~IStreamBase() = default;

    virtual size_t read(char* buf, size_t size) const = 0;
    virtual size_t write(const char* buf, size_t size) const = 0;
    virtual void seek(int64_t offset, SeekMode mode) const = 0;
    virtual size_t tell() const = 0;
    virtual size_t size() const = 0;
};
