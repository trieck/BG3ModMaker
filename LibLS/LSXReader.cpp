#include "pch.h"
#include "LSCommon.h"
#include "LSXReader.h"

#include "Exception.h"
#include "XmlWrapper.h"

LSXReader::LSXReader()
{
}

LSXReader::~LSXReader()
{
}

Resource::Ptr LSXReader::read(const ByteBuffer& info)
{
    m_resource = std::make_unique<Resource>();

    XmlWrapper xmlDoc(info);

    auto root = xmlDoc.doc().root();

    for_each(root);

    auto resource = std::move(m_resource);

    return resource;
}

bool LSXReader::begin(pugi::xml_node& node)
{
    if (node.type() != pugi::node_element) {
        return true;
    }

    auto name = node.name();
    if (strcmp(name, "header") == 0) {
        m_resource->metadata.timeStamp = node.attribute("timestamp").as_ullong();
    } else if (strcmp(name, "version") == 0) {
        m_resource->metadata.majorVersion = node.attribute("major").as_uint();
        m_resource->metadata.minorVersion = node.attribute("minor").as_uint();
        m_resource->metadata.revision = node.attribute("revision").as_uint();
        m_resource->metadata.buildNumber = node.attribute("build").as_uint();

        auto version = m_resource->metadata.majorVersion >= 4 ? LSXVersion::V4 : LSXVersion::V3;
        // TODO: lslibMeta
        // SerializationSettings.InitFromMeta(lslibMeta ?? "");
        // resource->metadataFormat = SerializationSettings.LSFMetaData;
    } else if (strcmp(name, "region") == 0) {
        m_region = std::make_shared<Region>();
        m_region->regionName = node.attribute("id").as_string();
        m_resource->regions[m_region->regionName] = m_region;
    } else if (strcmp(name, "node") == 0) {
        if (!m_region) {
            throw Exception("A <node> must be located inside a region.");
        }

        LSNode::Ptr lsNode;
        if (m_stack.empty()) {
            lsNode = m_region;
        } else {
            lsNode = std::make_shared<LSNode>();
            lsNode->parent = m_stack.back();
        }

        lsNode->name = node.attribute("id").as_string();
        if (auto parent = lsNode->parent.lock()) {
            parent->appendChild(lsNode);
        }

        lsNode->keyAttribute = node.attribute("key").as_string();
        m_stack.emplace_back(lsNode);
    } else if (strcmp(name, "attribute") == 0) {
        auto typeId = AttributeTypeMaps::typeToId(node.attribute("type").as_string());
        auto attrName = node.attribute("id").as_string();

        NodeAttribute attr(typeId);

        auto nodeValue = node.attribute("value");
        if (typeId == TranslatedString) {
            // TODO: support translated strings
            ASSERT(0);
        } else if (typeId == TranslatedFSString) {
            // TODO: support translated FS strings
            ASSERT(0);
        } else {
            attr.fromString(nodeValue.as_string());
        }

        m_stack.back()->attributes[attrName] = std::move(attr);
    }

    return true;
}

bool LSXReader::end(pugi::xml_node& node)
{
    if (node.type() != pugi::node_element) {
        return true;
    }

    auto name = node.name();

    if (strcmp(name, "node") == 0) {
        if (m_stack.empty()) {
            throw Exception("Unmatched <node> end tag.");
        }
        m_stack.pop_back();
    } else if (strcmp(name, "region") == 0) {
        m_region.reset();
    }

    return true;
}

bool LSXReader::for_each(pugi::xml_node& node)
{
    begin(node);

    for (auto& child : node.children()) {
        for_each(child);
    }

    end(node);

    return true;
}
