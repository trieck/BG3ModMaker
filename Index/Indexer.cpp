
#include "stdafx.h"

#include "Indexer.h"
#include "XmlWrapper.h"


#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <rocksdb/db.h>

Indexer::Indexer()
= default;

void Indexer::index(const char* pakFile)
{
    m_reader.read(pakFile);

    auto buffer = m_reader.readFile("Public/Tom/RootTemplates/merged.lsx");

    XmlWrapper xmlDoc(buffer);

    auto gameObjects = xmlDoc.selectNodes("//region[@id='Templates']//node[@id='Templates']//children//node[@id='GameObjects']");
    for (auto gameObject : gameObjects) {

        auto gameObjectNode = gameObject.node();

        std::cout << "GameObject: id=" << gameObjectNode.attribute("id").as_string() << std::endl;

        for (auto attribute: gameObjectNode.children("attribute")) {

            auto id = attribute.attribute("id").as_string();
            auto type = attribute.attribute("type").as_string();
            auto handle = attribute.attribute("handle").as_string();
            auto version = attribute.attribute("version").as_string();
            auto value = attribute.attribute("value").as_string();

            std::cout << "    Attribute: id=" << id << ", type=" << type << ", handle=" << handle
                << ", version=" << version << ", value=" << value << std::endl;
        }
    }
}
