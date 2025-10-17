#pragma once

enum class SeekMode : DWORD
{
    Begin = FILE_BEGIN,
    Current = FILE_CURRENT,
    End = FILE_END
};

template <typename T>
concept StreamReadable = std::is_trivially_copyable_v<T>;

template <typename T>
concept StreamWritable = std::is_trivially_copyable_v<T>;

class StreamBase
{
public:
    virtual ~StreamBase() = default;
    virtual size_t read(char* buf, size_t size) = 0;
    virtual size_t write(const char* buf, size_t size) = 0;
    virtual void seek(int64_t offset, SeekMode mode) = 0;
    virtual size_t tell() const = 0;
    virtual size_t size() const = 0;

    template <typename T>
    T read() requires (StreamReadable<T>);

    template <typename T>
    size_t read(T* data, size_t count) requires (StreamReadable<T>);

    template <typename T>
    size_t write(T value) requires (StreamWritable<T>);

    template <typename T>
    size_t write(const T* data, size_t count) requires (StreamWritable<T>);
};

template <typename T>
T StreamBase::read() requires (StreamReadable<T>)
{
    T value;
    read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

template <typename T>
size_t StreamBase::read(T* data, size_t count) requires (StreamReadable<T>)
{
    return read(reinterpret_cast<char*>(data), sizeof(T) * count);
}

template <typename T>
size_t StreamBase::write(T value) requires (StreamWritable<T>)
{
    return write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
size_t StreamBase::write(const T* data, size_t count) requires (StreamWritable<T>)
{
    return write(reinterpret_cast<const char*>(data), sizeof(T) * count);
}
