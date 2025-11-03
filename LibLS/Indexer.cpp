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
    if (text.length() < 2) {
        return "";
    }

    // Handle empty GUID
    if (text == "00000000-0000-0000-0000-000000000000") {
        return "";
    }

    std::string lower = text;
    std::ranges::transform(lower, lower.begin(), tolower);

    std::vector<std::string> tokens;
    std::smatch m;

    static const std::regex handleRegex(
        "(h(?:[0-9A-Fa-f]{32}|[0-9A-Fa-f]{8}g[0-9A-Fa-f]{4}g[0-9A-Fa-f]{4}g[0-9A-Fa-f]{4}g[0-9A-Fa-f]{12}))(?:;\\d+)?");

    if (std::regex_match(lower, m, handleRegex)) {
        tokens.emplace_back(m[1].str());
    } else if (std::regex_match(
        lower, std::regex(R"(^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$)"))) {
        tokens.emplace_back(lower); // full form
        std::istringstream ss(lower);
        std::string part;
        while (std::getline(ss, part, '-')) {
            if (!part.empty()) {
                tokens.emplace_back(part);
            }
        }
    } else if (std::regex_match(text, std::regex(R"(^[A-Za-z0-9_]+$)"))
        && text.find('_') != std::string::npos) {
        tokens.emplace_back(lower); // full form

        // Split on underscores first
        std::istringstream ss(text);
        std::string part;
        while (std::getline(ss, part, '_')) {
            if (part.size() <= 1) {
                continue;
            }

            // Now split case *within each underscore part*
            if (std::regex_search(part, std::regex("([a-z][A-Z])"))) {
                std::string split = std::regex_replace(part, std::regex("([a-z])([A-Z])"), "$1 $2");
                std::ranges::transform(split, split.begin(), tolower);
                std::istringstream camel(split);
                std::string w;
                while (camel >> w) {
                    if (w.size() > 1) {
                        tokens.emplace_back(w);
                    }
                }
            } else if (part.size() > 1) {
                std::string low = part;
                std::ranges::transform(low, low.begin(), tolower);
                tokens.emplace_back(low);
            }
        }
    } else if (text.find(' ') == std::string::npos &&
        std::regex_search(text, std::regex("([a-z][A-Z])"))) {
        std::string split = std::regex_replace(text, std::regex("([a-z])([A-Z])"), "$1 $2");
        std::ranges::transform(split, split.begin(), tolower);
        tokens.emplace_back(lower); // compact form

        std::istringstream ss(split);
        std::string word;
        while (ss >> word) {
            if (word.size() > 1) {
                tokens.emplace_back(word);
            }
        }
    } else {
        std::istringstream ss(lower);

        std::string word;
        while (ss >> word) {
            if (word.size() > 1) {
                tokens.emplace_back(word);
            }
        }
    }

    std::ostringstream out;
    for (size_t i = 0; i < tokens.size(); ++i) {
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

    auto i = 0u;
    const auto count = terms.size();

    for (const auto& term : terms) {
        values << term;
        if (++i < count) {
            values << ' ';
        }
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
            m_listener->onFile(i, file.name);
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

void Indexer::setProgressListener(IFileProgressListener* listener)
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

            attributes.emplace_back(std::move(attr));
        }

        if (terms.empty()) {
            continue;
        }

        doc["attributes"] = attributes;

        Xapian::Document xdoc;
        m_termgen.set_document(xdoc);
        m_termgen.index_text(termsToString(terms));
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
    std::regex reUsing(R"REG(^[ \t]*using[ \t]+"([^"]+)")REG",
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
        } else if (std::regex_search(line, m, reUsing)) {
            json attr;
            attr["id"] = "Using";
            attr["value"] = m[1].str();
            attr["type"] = "Using";
            attributes.emplace_back(std::move(attr));

            addTerms(terms, m[1].str());
        } else if (std::regex_search(line, m, reData)) {
            json attr;
            auto id = m[1].str();
            auto value = m[2].str();

            attr["id"] = id;
            attr["value"] = value;
            attr["type"] = "data";
            attributes.emplace_back(std::move(attr));

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
