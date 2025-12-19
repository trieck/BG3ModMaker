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
    bool shortTypeIds() const;

    OsiReader(OsiReader&&) noexcept;
    OsiReader& operator=(OsiReader&&) noexcept;
    OsiReader(const OsiReader&) = delete;
    OsiReader& operator=(const OsiReader&) = delete;

    bool readFile(const char* filename);
    std::string readString();
    OsiVersion version() const;

    // StreamBase
    size_t read(char* buf, size_t size) override;
    size_t write(const char* buf, size_t size) override;
    void seek(int64_t offset, SeekMode mode) override;
    size_t tell() const override;
    size_t size() const override;

private:
    OsiDivObject readDivObject();
    OsiEnum readEnum();
    OsiFunction readFunction();
    OsiFunctionSig readFunctionSig();

    OsirisType readType();
    std::vector<std::string> readStrings();
    void makeBuiltins();

    void readDivObjects();
    void readEnums();
    void readFunctions();
    void readNodes();
    OsiNode::Ptr readNode();
    void readStringTable();
    void readTypes();

    FileStream m_file{};
    Story m_story;
    uint8_t m_scramble = 0x00;
    bool m_shortTypeIds = false;
};
