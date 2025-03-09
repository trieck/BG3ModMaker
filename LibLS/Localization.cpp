#include "pch.h"

#include "Exception.h"
#include "FileStream.h"
#include "Localization.h"
#include "XmlWrapper.h"

LocaResource LocaReader::Read(const std::string& path)
{
    LocaResource resource;

    FileStream stream;
    stream.open(path.c_str(), "rb");

    auto header = stream.read<LocaHeader>();
    if (header.signature != LOCA_SIGNATURE) {
        throw Exception("Invalid localization signature.");
    }

    auto entries = std::make_unique<LocaEntry[]>(header.numEntries);

    stream.read<LocaEntry>(entries.get(), header.numEntries);

    if (stream.tell() != header.textsOffset) {
        stream.seek(header.textsOffset, SeekMode::Begin);
    }

    for (auto i = 0ul; i < header.numEntries; i++) {
        const auto& entry = entries[i];
        auto text = stream.read(entry.length - 1).str();

        LocalizedText localizedText;
        localizedText.key = reinterpret_cast<const char*>(entry.key);
        localizedText.version = entry.version;
        localizedText.text = text;
    }

    return resource;
}

void LocaWriter::Write(const std::string& path, const LocaResource& resource)
{
    FileStream stream;
    stream.open(path.c_str(), "wb");

    LocaHeader header;
    header.signature = LOCA_SIGNATURE;
    header.numEntries = static_cast<uint32_t>(resource.entries.size());
    header.textsOffset = static_cast<uint32_t>(sizeof(LocaHeader) + sizeof(LocaEntry) * resource.entries.size());

    stream.write(header);

    auto entries = std::make_unique<LocaEntry[]>(header.numEntries);
    for (auto i = 0ul; i < resource.entries.size(); i++) {
        const auto& entry = resource.entries[i];
        auto& locaEntry = entries[i];
        memcpy(locaEntry.key, entry.key.c_str(), entry.key.size());
        locaEntry.version = entry.version;
        locaEntry.length = static_cast<uint32_t>(entry.text.size() + 1);
    }

    stream.write<LocaEntry>(entries.get(), header.numEntries);

    for (const auto& entry : resource.entries) {
        stream.write(entry.text.c_str(), entry.text.size());
        stream.write('\0');
    }
}

LocaResource LocaXmlReader::Read(const std::string& path)
{
    LocaResource resource;

    FileStream stream;
    stream.open(path.c_str(), "rb");

    auto buffer = stream.read(stream.size()).detach();

    auto xml = XmlWrapper(buffer);

    auto nodes = xml.selectNodes("//contentList/content");
    for (const auto& node : nodes) {
        auto content = node.node();

        LocalizedText localizedText;
        localizedText.key = content.attribute("contentuid").as_string();

        localizedText.version = 1;
        if (!content.attribute("version").empty()) {
            localizedText.version = static_cast<uint16_t>(content.attribute("version").as_uint());
        }

        localizedText.text = content.child_value();

        resource.entries.emplace_back(std::move(localizedText));
    }

    return resource;
}
