#pragma once

#include <format>

enum class SeekMode : DWORD
{
    Begin = FILE_BEGIN,
    Current = FILE_CURRENT,
    End = FILE_END
};

template <typename T>
concept StreamBinary =
    std::is_trivially_copyable_v<T> &&
    std::is_standard_layout_v<T> &&
    !std::is_pointer_v<T> &&
    !std::is_reference_v<T> &&
    !std::is_array_v<T>;

template <typename T>
concept TextNumber = std::is_arithmetic_v<T> &&
    !std::is_same_v<T, bool>;

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
    T peek() requires (StreamBinary<T>);

    template <typename T>
    T read() requires (StreamBinary<T>);

    template <typename T>
    size_t read(T* data, size_t count) requires (StreamBinary<T>);

    template <typename T>
    size_t write(const T& value) requires (StreamBinary<T>);

    size_t write(std::string_view value);

    template <typename T>
    size_t writeText(const T& value) requires (TextNumber<T>);

    size_t writeText(bool b);

    template <typename T>
    size_t write(const T* data, size_t count) requires (StreamBinary<T>);
};

template <typename T>
T StreamBase::peek() requires (StreamBinary<T>)
{
    T value;
    read(reinterpret_cast<char*>(&value), sizeof(T));
    seek(-static_cast<int64_t>(sizeof(T)), SeekMode::Current);
    return value;
}

template <typename T>
T StreamBase::read() requires (StreamBinary<T>)
{
    T value;
    read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

template <typename T>
size_t StreamBase::read(T* data, size_t count) requires (StreamBinary<T>)
{
    return read(reinterpret_cast<char*>(data), sizeof(T) * count);
}

template <typename T>
size_t StreamBase::write(const T& value) requires (StreamBinary<T>)
{
    return write(reinterpret_cast<const char*>(&value), sizeof(T));
}

inline size_t StreamBase::write(std::string_view value)
{
    return write(value.data(), value.size());
}

inline size_t StreamBase::writeText(bool b)
{
    std::string str = b ? "true" : "false";
    return write(str);
}

template <typename T>
size_t StreamBase::writeText(const T& value) requires (TextNumber<T>)
{
    auto str = std::format("{}", value);
    return write(str);
}

template <typename T>
size_t StreamBase::write(const T* data, size_t count) requires (StreamBinary<T>)
{
    return write(reinterpret_cast<const char*>(data), sizeof(T) * count);
}
