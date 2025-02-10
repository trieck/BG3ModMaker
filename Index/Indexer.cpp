
#include "stdafx.h"

#include <nlohmann/json.hpp>
#include <unordered_set>


#pragma warning(push)
#pragma warning(disable : 4996)  // Suppress deprecation warnings
#include <xapian.h>
#pragma warning(pop)

#include "Indexer.h"
#include "XmlWrapper.h"


using json = nlohmann::json;

const std::unordered_set<std::string> STOP_WORDS = {
    "the", "and", "of", "to", "in", "for", "with", "on", "at", "by"
};

static std::string normalizeText(std::string text) {
    std::replace(text.begin(), text.end(), '_', ' ');  // Replace _ with space
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);  // Convert to lowercase

    std::istringstream stream(text);
    std::ostringstream filtered;
    std::string word;
    while (stream >> word) {
        if (STOP_WORDS.find(word) == STOP_WORDS.end()) {  // Keep only non-stop words
            filtered << word << " ";
        }
    }
    return filtered.str();
}

Indexer::Indexer()
= default;

void Indexer::index(const char* pakFile, const char* dbName)
{
    m_reader.read(pakFile);

    auto lsxFile = "Public/Tom/RootTemplates/merged.lsx";
    auto buffer = m_reader.readFile(lsxFile);

    Xapian::WritableDatabase db(dbName, Xapian::DB_CREATE_OR_OPEN);
    Xapian::TermGenerator termgen;

    Xapian::SimpleStopper stopper;
    for (const auto& stopWord : STOP_WORDS) {
        stopper.add(stopWord);
    }
    termgen.set_stopper(&stopper);

    XmlWrapper xmlDoc(buffer);
       
    auto gameObjects = xmlDoc.selectNodes("//region[@id='Templates']//node[@id='Templates']//children//node[@id='GameObjects']");
    for (auto gameObject : gameObjects) {

        auto gameObjectNode = gameObject.node();

        auto nodeId = gameObjectNode.attribute("id").as_string();

        json gameObjectJson;
        gameObjectJson["source_file"] = lsxFile;
        gameObjectJson["type"] = nodeId;

        json attributesArray = json::array();

        std::string uuid;

        for (auto attribute: gameObjectNode.children("attribute")) {

            json attributeJson;

            for (auto attr : attribute.attributes()) {
                std::string key = attr.name();
                std::string value = attr.as_string();

                if (!value.empty()) {
                    attributeJson[key] = value;
                }
            }

            if (attributeJson["id"] == "MapKey") {
                uuid = attributeJson["value"];
            }

            if (!attributeJson.empty()) {
                attributesArray.push_back(attributeJson);
            }
        }

        if (attributesArray.empty()) {
            continue;
        }

        gameObjectJson["attributes"] = attributesArray;

        if (!uuid.empty()) {
            gameObjectJson["uuid"] = uuid;

            std::string values;
            for (auto& [key, value] : gameObjectJson.items()) {

                if (key == "attributes") {
                    for (auto& attribute : value) {
                        for (auto& [attrKey, attrValue] : attribute.items()) {
                            values += normalizeText(attrValue.get<std::string>()) + " ";
                        }
                    }
                    continue;
                }

                values += normalizeText(value.get<std::string>()) + " ";
            }

            Xapian::Document doc;
            termgen.set_document(doc);
            termgen.index_text(values);
            doc.set_data(gameObjectJson.dump());

            db.add_document(doc);
            db.commit();
        }
    }
}
