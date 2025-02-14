#include "pch.h"

#include <nlohmann/json.hpp>
#include <unordered_set>
#include <regex>
#include <xapian.h>

#include "Indexer.h"

#include "LSFReader.h"
#include "Utility.h"
#include "XmlWrapper.h"

using json = nlohmann::json;

namespace {
    const std::unordered_set<std::string> STOP_WORDS = {
        "the", "and", "of", "to", "in", "for", "with", "on", "at", "by"
    };

    std::string normalizeText(std::string text)
    {
        text = std::regex_replace(text, std::regex("[^a-zA-Z0-9-]"), " ");

        std::ranges::transform(text, text.begin(), ::tolower);

        std::istringstream stream(text);
        std::ostringstream filtered;
        std::string word;
        while (stream >> word) {
            if (!STOP_WORDS.contains(word)) {
                filtered << word << " ";
            }
        }

        return filtered.str();
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

} // anonymous namespace

Indexer::Indexer()
{
    for (const auto& stopWord : STOP_WORDS) {
        m_stopper.add(stopWord);
    }

    m_termgen.set_stopper(&m_stopper);
    m_termgen.set_stopper_strategy(Xapian::TermGenerator::STOP_ALL);
}

void Indexer::index(const char* pakFile, const char* dbName)
{
    std::cout << std::format("Indexing {} into {}\n", pakFile, dbName);
    std::cout << "   Reading PAK file...";
    m_reader.read(pakFile);
    std::cout << "done" << std::endl;

    m_db = std::make_unique<Xapian::WritableDatabase>(dbName, Xapian::DB_CREATE_OR_OVERWRITE);

    auto i = 0;
    for (const auto& file : m_reader.files()) {
        if (file.name.ends_with("lsx")) {
            std::cout << std::format("Indexing {} ({}/{})\n", file.name, i, m_reader.files().size());
            indexLSXFile(file);
        } else if (file.name.ends_with("lsf")) {
            std::cout << std::format("Indexing {} ({}/{})\n", file.name, i, m_reader.files().size());
            indexLSFFile(file);
        } else {
            std::cout << std::format("Skipping {} ({}/{})\n", file.name, i, m_reader.files().size());
        }

        if (++i % 10000 == 0) {
            m_db->commit();
        }
    }

    std::cout << "   Committing changes to database...";
    m_db->commit();
    std::cout << "done" << std::endl;

    std::cout << "   Indexed " << comma(m_db->get_doccount()) << " documents" << std::endl;
}

void Indexer::compact() const
{
    if (m_db) {
        m_db->compact(Xapian::DBCOMPACT_SINGLE_FILE);
    }
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
        xdoc.set_data(doc.dump());
        m_db->add_document(xdoc);
    }
}

void Indexer::indexNode(const std::string& filename, const Node::Ptr& node)
{
    std::unordered_set<std::string> terms;

    json doc;
    doc["source_file"] = filename;
    doc["type"] = node->name;

    json attributes = json::array();
    for (const auto& [key, val] : node->attributes) {
        json attr;

        attr["id"] = key;
        attr["value"] = val.value();
        attr["type"] = val.typeStr();

        if (key == "Script") {
            continue; // Skip script content
        }

        addTerms(terms, val.value());

        attributes.push_back(attr);
    }

    doc["attributes"] = attributes;

    if (terms.empty()) {
        return;
    }

    std::ostringstream values;
    for (const auto& term : terms) {
        values << term << " ";
    }

    Xapian::Document xdoc;
    m_termgen.set_document(xdoc);
    m_termgen.index_text(values.str());
    xdoc.set_data(doc.dump());
    m_db->add_document(xdoc);
}

void Indexer::indexNodes(const std::string& filename, const std::vector<Node::Ptr>& nodes)
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

void Indexer::indexLSFFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);

    LSFReader reader;
    auto resource = reader.read(buffer);

    for (const auto& val : resource->regions | std::views::values) {
        indexRegion(file.name, val);
    }
}
