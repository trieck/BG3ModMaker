#pragma once

class FileStream
{
public:
    FileStream();
    ~FileStream();
    void Open(const char* path, const char* mode);
    void Close();
    void Write(const void* data, size_t size) const;
    void Read(void* data, size_t size) const;
    void Seek(int64_t offset, SeekMode mode) const;
    size_t Tell() const;
    size_t Size() const;
    bool IsOpen() const;

private:
    HANDLE m_file;
};

