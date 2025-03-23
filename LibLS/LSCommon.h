#pragma once

enum CompressionFlags : uint8_t {
    METHOD_NONE = 0x00,
    METHOD_ZLIB = 0x01,
    METHOD_LZ4 = 0x02,
    METHOD_LZSTD = 0x03,
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

    static PackedVersion fromInt64(int64_t packed) {
        return {
            .major= static_cast<uint32_t>(packed >> 55) & 0x7F,
            .minor= static_cast<uint32_t>(packed >> 47 & 0xFF),
            .revision= static_cast<uint32_t>(packed >> 31 & 0xFFFF),
            .build= static_cast<uint32_t>(packed & 0x7FFFFFFF)
        };
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


