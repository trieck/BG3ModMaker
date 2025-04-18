#pragma once

// Computes a 32-bit FNV-1a hash for any trivially copyable type.
// - Guaranteed stable across platforms and STL versions.
// - Prefer this over std::hash for serialization or persistent file formats.
// - Accepts raw data as std::span<const std::byte> or any trivially copyable type.

namespace fnvhash {

constexpr uint32_t fnv_offset_basis = 2166136261u;
constexpr uint32_t fnv_prime = 16777619u;

constexpr uint32_t fnv1a_hash(std::span<const std::byte> data)
{
    uint32_t hash = fnv_offset_basis;

    for (auto b : data) {
        hash ^= static_cast<uint8_t>(b);
        hash *= fnv_prime;
    }

    return hash;
}

template <typename T>
    requires std::is_trivially_copyable_v<T>
constexpr uint32_t hash(const T& value)
{
    auto ptr = reinterpret_cast<const std::byte*>(&value);
    return fnv1a_hash(std::span(ptr, sizeof(T)));
}

inline uint32_t hash(const char* str)
{
    // C-style null-terminated string, excluding the null terminator
    auto len = strlen(str);

    return fnv1a_hash(std::span(
        reinterpret_cast<const std::byte*>(str), len));
}

inline uint32_t hash(const std::string& str)
{
    return fnv1a_hash(std::as_bytes(std::span(str.data(), str.size())));
}

inline uint32_t hash(std::string_view str)
{
    return fnv1a_hash(std::as_bytes(std::span(str.data(), str.size())));
}

} // namespace fnvhash
