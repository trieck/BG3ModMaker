#pragma once
#include "FileStream.h"
#include "Story.h"

class OsiReader : public StreamBase
{
public:
    using StreamBase::read;
    using StreamBase::write;

    OsiReader();
    ~OsiReader() override;
    
    OsiReader(OsiReader&&) noexcept;
    OsiReader& operator=(OsiReader&&) noexcept;
    OsiReader(const OsiReader&) = delete;
    OsiReader& operator=(const OsiReader&) = delete;

    bool getEnum(uint16_t type, OsiEnum& osi_enum);
    bool isAlias(uint32_t type) const;
    bool readFile(const char* filename);
    bool shortTypeIds() const;
    OsiValueType resolveAlias(OsiValueType type);
    OsiVersion version() const;
    std::string readString();

    // StreamBase
    size_t read(char* buf, size_t size) override;
    size_t write(const char* buf, size_t size) override;
    void seek(int64_t offset, SeekMode mode) override;
    size_t tell() const override;
    size_t size() const override;

private:
    OsiNode::Ptr readNode();
    OsiType makeBuiltin(const std::string& name, uint8_t index) const;
    std::vector<std::string> readStrings();
    void makeBuiltins();
    void readAdapters();
    void readDatabases();
    void readDivObjects();
    void readEnums();
    void readFunctions();
    void readGlobalActions();
    void readGoals();
    void readNodes();
    void readStringTable();
    void readTypes();

    FileStream m_file{};
    Story m_story;
    uint8_t m_scramble = 0x00;
    bool m_shortTypeIds = false;
};
