#pragma once

enum CompressionFlags : uint8_t
{
    METHOD_NONE = 0x00,
    METHOD_ZLIB = 0x01,
    METHOD_LZ4 = 0x02,
    METHOD_ZSTD = 0x03,
    FAST_COMPRESS = 0x10,
    DEFAULT_COMPRESS = 0x20,
    MAX_COMPRESS = 0x40
};

struct PackedVersion
{
    uint32_t major;
    uint32_t minor;
    uint32_t revision;
    uint32_t build;

    static PackedVersion fromInt64(int64_t packed)
    {
        return {
            .major = static_cast<uint32_t>(packed >> 55) & 0x7F,
            .minor = static_cast<uint32_t>(packed >> 47 & 0xFF),
            .revision = static_cast<uint32_t>(packed >> 31 & 0xFFFF),
            .build = static_cast<uint32_t>(packed & 0x7FFFFFFF)
        };
    }

    int32_t toVersion32()
    {
        return (static_cast<int32_t>(major & 0x0f) << 28) |
            (static_cast<int32_t>(minor & 0x0f) << 24) |
            (static_cast<int32_t>(revision & 0xff) << 16) |
            static_cast<int32_t>(build & 0xffff);
    }

    int64_t toVersion64()
    {
        return (static_cast<int64_t>(major & 0x7f) << 55) |
            (static_cast<int64_t>(minor & 0xff) << 47) |
            (static_cast<int64_t>(revision & 0xffff) << 31) |
            static_cast<int64_t>(build & 0x7fffffff);
    }
};

struct LSMetadata
{
    static constexpr uint32_t currentMajorVersion = 33;

    uint64_t timeStamp;
    uint32_t majorVersion;
    uint32_t minorVersion;
    uint32_t revision;
    uint32_t buildNumber;
};

enum class LSXVersion : uint32_t
{
    V3 = 3,
    V4 = 4
};
