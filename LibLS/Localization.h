#pragma once

static auto constexpr LOCA_SIGNATURE = 0x41434F4C; // LOCA

#pragma pack(push, 1)

struct LocaHeader
{
    uint32_t signature;
    uint32_t numEntries;
    uint32_t textsOffset;
};

struct LocaEntry
{
    uint8_t key[64];
    uint16_t version;
    uint32_t length;
};

#pragma pack(pop)

struct LocalizedText
{
    std::string key;
    uint16_t version;
    std::string text;
};

struct LocaResource
{
    std::vector<LocalizedText> entries;
};

class LocaReader
{
public:
    static LocaResource Read(const std::string& path);
};

class LocaWriter
{
public:
    static void Write(const std::string& path, const LocaResource& resource);
};

class LocaXmlReader
{
public:
    static LocaResource Read(const std::string& path);
};
