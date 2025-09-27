#include "pch.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <regex>
#include <unordered_set>
#include <xapian.h>

#include "Indexer.h"
#include "LSFReader.h"
#include "XmlWrapper.h"

using json = nlohmann::json;

static constexpr auto COMMIT_SIZE = 1000;

namespace {
const std::unordered_set<std::string> STOP_WORDS = {
    "the", "and", "of", "to", "in", "for", "with", "on", "at", "by"
};

std::string normalizeText(const std::string& text)
{
    std::string lowered = text;

    // only apply case/digit splitting to non-hex strings
    if (!std::regex_match(lowered, std::regex("^[A-Fa-f0-9\\-]+$"))) {
        // insert spaces at case-change and letter-digit boundaries
        lowered = std::regex_replace(lowered, std::regex("([a-z])([A-Z])"), "$1 $2");
        lowered = std::regex_replace(lowered, std::regex("([A-Za-z])([0-9])"), "$1 $2");
        lowered = std::regex_replace(lowered, std::regex("([0-9])([A-Za-z])"), "$1 $2");
    }

    // lowercase everything
    std::ranges::transform(lowered, lowered.begin(), tolower);

    std::vector<std::string> tokens;

    // split form
    auto splitText = std::regex_replace(lowered, std::regex("[^a-z0-9]"), " ");
    std::istringstream stream(splitText);

    std::string word;
    while (stream >> word) {
        if (!STOP_WORDS.contains(word) && word.size() > 1) {
            tokens.push_back(word);
        }
    }

    // whole form
    if (std::regex_search(text, std::regex("[_-]"))) {
        auto whole = std::regex_replace(text, std::regex("[^A-Za-z0-9_\\-]"), "");
        auto wholeLower = whole;
        std::ranges::transform(wholeLower, wholeLower.begin(), tolower);
        if (wholeLower.size() > 1 && !STOP_WORDS.contains(wholeLower)) {
            tokens.push_back(wholeLower);
        }
    }

    // join
    std::ostringstream out;
    for (auto i = 0u; i < tokens.size(); ++i) {
        if (i > 0) {
            out << ' ';
        }
        out << tokens[i];
    }

    return out.str();
}

void addTerms(std::unordered_set<std::string>& terms, const std::string& value)
{
    auto normalized = normalizeText(value);

    std::istringstream stream(normalized);

    std::string word;
    while (stream >> word) {
        if (word.size() > 1) {
            terms.insert(word);
        }
    }
}

std::string termsToString(const std::unordered_set<std::string>& terms)
{
    std::ostringstream values;
    for (const auto& term : terms) {
        values << term << " ";
    }

    return values.str();
}
} // anonymous namespace

Indexer::Indexer()
{
    for (const auto& stopWord : STOP_WORDS) {
        m_stopper.add(stopWord);
    }

    m_termgen.set_stopper(&m_stopper);
    m_termgen.set_stopper_strategy(Xapian::TermGenerator::STOP_ALL);
}

void Indexer::index(const char* pakFile, const char* dbName, bool overwrite)
{
    m_reader.read(pakFile);

    if (m_listener) {
        m_listener->onStart(m_reader.files().size());
    }

    auto flags = overwrite ? Xapian::DB_CREATE_OR_OVERWRITE : Xapian::DB_CREATE_OR_OPEN;

    m_db = std::make_unique<Xapian::WritableDatabase>(dbName, flags);

    static constexpr auto extensions = std::array{".lsx", ".lsf", ".txt"};

    auto i = 0;
    for (const auto& file : m_reader.files()) {
        if (m_listener && m_listener->isCancelled()) {
            break;
        }

        if (std::ranges::none_of(extensions, [&](const auto& ext) { return file.name.ends_with(ext); })) {
            ++i;
            continue;
        }

        if (m_listener) {
            m_listener->onFileIndexing(i, file.name);
        }

        if (file.name.ends_with(".lsx")) {
            indexLSXFile(file);
        } else if (file.name.ends_with(".lsf")) {
            indexLSFFile(file);
        } else if (file.name.ends_with(".txt")) {
            indexTXTFile(file);
        }

        if (++i % COMMIT_SIZE == 0) {
            m_db->commit();
        }
    }

    m_db->commit();

    if (m_listener) {
        if (m_listener->isCancelled()) {
            m_listener->onCancel();
        } else {
            m_listener->onFinished(m_db->get_doccount());
        }
    }
}

void Indexer::compact() const
{
    if (m_db) {
        // m_db->compact(Xapian::DBCOMPACT_SINGLE_FILE);
    }
}

void Indexer::setProgressListener(IIndexProgressListener* listener)
{
    m_listener = listener;
}

void Indexer::indexLSXFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);
    XmlWrapper xmlDoc(buffer);

    auto nodes = xmlDoc.selectNodes("//*[self::node]");

    for (const auto& xpathNode : nodes) {
        std::unordered_set<std::string> terms;

        auto node = xpathNode.node();

        json doc;
        doc["source_file"] = file.name;
        doc["type"] = node.attribute("id").value();

        if (node.attributes().empty()) {
            continue;
        }

        json attributes = json::array();
        for (const auto& attribute : node.children("attribute")) {
            std::string id = attribute.attribute("id").as_string();
            std::string type = attribute.attribute("type").as_string();
            std::string value = attribute.attribute("value").as_string();

            if (id.empty() || value.empty()) {
                continue;
            }

            json attr;
            attr["id"] = id;
            attr["value"] = value;
            attr["type"] = type;

            if (id == "Script") {
                continue; // Skip script content
            }

            addTerms(terms, value);

            attributes.push_back(attr);
        }

        if (terms.empty()) {
            continue;
        }

        std::ostringstream values;
        for (const auto& term : terms) {
            values << term << " ";
        }

        doc["attributes"] = attributes;

        Xapian::Document xdoc;
        m_termgen.set_document(xdoc);
        m_termgen.index_text(values.str());
        xdoc.set_data(doc.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
        m_db->add_document(xdoc);
    }
}

void Indexer::indexNode(const std::string& filename, const LSNode::Ptr& node)
{
    std::unordered_set<std::string> terms;

    json doc;
    doc["source_file"] = filename;
    doc["type"] = node->name;

    json attributes = json::array();
    for (const auto& [key, val] : node->attributes) {
        json attr;

        attr["id"] = key;
        attr["value"] = val.str();
        attr["type"] = val.typeStr();

        if (key == "Script") {
            continue; // Skip script content
        }

        addTerms(terms, val.str());

        attributes.emplace_back(std::move(attr));
    }

    doc["attributes"] = attributes;

    if (terms.empty()) {
        return;
    }

    Xapian::Document xdoc;
    m_termgen.set_document(xdoc);
    m_termgen.index_text(termsToString(terms));
    xdoc.set_data(doc.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
    m_db->add_document(xdoc);

    for (const auto& val : node->children | std::views::values) {
        for (const auto& childNode : val) {
            indexNode(filename, childNode);
        }
    }
}

void Indexer::indexNodes(const std::string& filename, const std::vector<LSNode::Ptr>& nodes)
{
    for (const auto& node : nodes) {
        indexNode(filename, node);
    }
}

void Indexer::indexRegion(const std::string& fileName, const Region::Ptr& region)
{
    for (const auto& val : region->children | std::views::values) {
        indexNodes(fileName, val);
    }
}

void Indexer::indexTXTFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);

    std::string text(buffer.first.get(), buffer.first.get() + buffer.second);

    std::regex reEntry(R"REG(^[ \t]*new entry[ \t]+"([^"]+)")REG",
                       std::regex_constants::icase);
    std::regex reType(R"REG(^[ \t]*type[ \t]+"?([^"]+)"?)REG",
                      std::regex_constants::icase);
    std::regex reData(R"REG(^[ \t]*data[ \t]+"([^"]+)"[ \t]+"([^"]+)")REG",
                      std::regex_constants::icase);

    std::istringstream stream(text);
    std::string line;
    std::string currentEntry, currentType;

    json attributes = json::array();
    std::unordered_set<std::string> terms;

    auto flush = [&] {
        if (!currentEntry.empty() && !currentType.empty()) {
            json doc;
            doc["source_file"] = file.name;
            doc["entry"] = currentEntry;
            doc["type"] = currentType;
            doc["attributes"] = attributes;

            terms.insert(currentEntry);

            Xapian::Document xdoc;
            m_termgen.set_document(xdoc);
            m_termgen.index_text(termsToString(terms));
            xdoc.set_data(doc.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
            m_db->add_document(xdoc);
        }

        currentEntry.clear();
        currentType.clear();
        attributes = json::array();
        terms.clear();
    };

    while (std::getline(stream, line)) {
        std::smatch m;
        if (std::regex_search(line, m, reEntry)) {
            flush();
            currentEntry = m[1].str();
        } else if (std::regex_search(line, m, reType)) {
            currentType = m[1];
        } else if (std::regex_search(line, m, reData)) {
            json attr;
            auto id = m[1].str();
            auto value = m[2].str();

            attr["id"] = id;
            attr["value"] = value;
            attr["type"] = "data";
            attributes.push_back(attr);

            addTerms(terms, id);
            addTerms(terms, value);
        }
    }

    flush(); // flush last block
}

void Indexer::indexLSFFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);

    LSFReader reader;
    auto resource = reader.read(buffer);

    for (const auto& val : resource->regions | std::views::values) {
        indexRegion(file.name, val);
    }
}
