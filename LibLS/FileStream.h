#pragma once

#include "Stream.h"
#include "StreamBase.h"

class FileStream : public StreamBase
{
public:
    using StreamBase::read;
    using StreamBase::write;

    FileStream();
    ~FileStream() override;

    void open(const char* path, const char* mode);
    void close();

    // IStreamBase
    size_t read(char* buf, size_t size) override;
    size_t write(const char* buf, size_t size) override;
    void seek(int64_t offset, SeekMode mode) override;
    size_t tell() const override;
    size_t size() const override;

    Stream read(size_t bytes);
    void write(const void* data, size_t size) const;
    bool isOpen() const;
private:
    HANDLE m_file;
};
