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
        std::ranges::replace(text, '_', ' ');
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
} // anonymous namespace

Indexer::Indexer()
{
    for (const auto& stopWord : STOP_WORDS) {
        m_stopper.add(stopWord);
    }
}

void Indexer::index(const char* pakFile, const char* dbName)
{
    m_reader.read(pakFile);

    m_db = std::make_unique<Xapian::WritableDatabase>(dbName, Xapian::DB_CREATE_OR_OVERWRITE);

    for (const auto& file : m_reader.files()) {
        if (file.name.ends_with("lsx")) {
            std::cout << "Indexing " << file.name << std::endl;
            indexLSXFile(file);
        } else if (file.name.ends_with("lsf")) {
            std::cout << "Indexing " << file.name << std::endl;
            indexLSFFile(file);
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

void Indexer::indexLSXFile(const PackagedFileInfo& file) const
{
    auto buffer = m_reader.readFile(file.name);
    XmlWrapper xmlDoc(buffer);

    auto nodes = xmlDoc.selectNodes("//region//node//children//node");
    for (const auto& xpathNode : nodes) {
        auto node = xpathNode.node();

        json doc;
        doc["source_file"] = file.name;
        doc["type"] = node.attribute("id").value();

        // TODO: we need to store region-node path like:
        //  <region id="Config">
        //    <node id="root">
        //     <children>
        //      <node id="ModuleInfo">

        if (node.attributes().empty()) {
            continue;
        }

        std::string uuid;
        json attributes = json::array();

        for (const auto& attribute : node.children("attribute")) {
            std::string id = attribute.attribute("id").as_string();
            std::string value = attribute.attribute("value").as_string();

            if (id.empty() || value.empty()) {
                continue;
            }

            json attr;
            attr[id] = value;

            // FIXME: This is a hack to find the UUID of the document
            if (id == "MapKey" || id == "UUID") {
                uuid = value;
            }

            if (id == "Script") {
                continue; // Skip script content
            }

            attributes.push_back(attr);
        }

        if (attributes.empty() || uuid.empty()) {
            continue;
        }

        doc["attributes"] = attributes;

        std::unordered_set<std::string> unique_terms;

        std::ostringstream values;
        for (auto& [key, value] : doc.items()) {
            if (key == "attributes") {
                for (auto& attribute : value) {
                    for (auto& [attrKey, attrValue] : attribute.items()) {
                        auto raw = attrValue.get<std::string>();
                        auto normalized = normalizeText(raw);

                        normalized = std::regex_replace(normalized, std::regex("[^a-zA-Z0-9-]"), " ");

                        std::istringstream iss(normalized);
                        std::string token;
                        while (iss >> token) { // Split into individual words
                            unique_terms.insert(token);
                            values << token << " ";
                        }
                    }
                }
            }
        }

        if (unique_terms.empty()) {
            continue;
        }

        Xapian::TermGenerator termgen;
        termgen.set_stopper(&m_stopper);
        termgen.set_stopper_strategy(Xapian::TermGenerator::STOP_ALL);

        Xapian::Document xdoc;
        termgen.set_document(xdoc);
        termgen.index_text(values.str());
        xdoc.set_data(doc.dump());
        m_db->add_document(xdoc);
    }
}

void Indexer::indexLSFFile(const PackagedFileInfo& file) const
{
    auto buffer = m_reader.readFile(file.name);

    LSFReader reader;
    reader.read(buffer);
}
