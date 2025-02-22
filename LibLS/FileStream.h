#pragma once

#include "IStreamBase.h"

class FileStream : public IStreamBase
{
public:
    FileStream();
    ~FileStream() override;

    void open(const char* path, const char* mode);
    void close();

    // IStreamBase
    size_t read(char* buf, size_t size) const override;
    size_t write(const char* buf, size_t size) const override;
    void seek(int64_t offset, SeekMode mode) const override;
    size_t tell() const override;
    size_t size() const override;

    void write(const void* data, size_t size) const;
    bool isOpen() const;
private:
    HANDLE m_file;
};

