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
    auto resource = std::make_unique<Resource>();

    XmlWrapper xmlDoc(info);

    auto nodes = xmlDoc.selectNodes("//*[self::node]");

    for (const auto& xpathNode : nodes) {
        std::unordered_set<std::string> terms;

        auto node = xpathNode.node();

        auto name = node.name();
        if (strcmp(name, "header") == 0) {
            resource->metadata.timeStamp = node.attribute("time").as_ullong();
        } else if (strcmp(name, "version") == 0) {
            resource->metadata.majorVersion = node.attribute("major").as_uint();
            resource->metadata.minorVersion = node.attribute("minor").as_uint();
            resource->metadata.revision = node.attribute("revision").as_uint();
            resource->metadata.buildNumber = node.attribute("build").as_uint();

            auto version = resource->metadata.majorVersion >= 4 ? LSXVersion::V4 : LSXVersion::V3;
            // TODO: lslibMeta
            // SerializationSettings.InitFromMeta(lslibMeta ?? "");
            // resource->metadataFormat = SerializationSettings.LSFMetaData;
        } else if (strcmp(name, "region") == 0) {
            m_region = std::make_shared<Region>();
            m_region->regionName = node.attribute("id").as_string();
            resource->regions[m_region->regionName] = m_region;
        } else if (strcmp(name, "node") == 0) {
            if (!m_region) {
                throw Exception("A <node> must be located inside a region.");
            }

            LSNode::Ptr lsNode;
            if (m_stack.empty()) {
                // The node is the root node of the region
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
                attr.setValue(nodeValue.as_string());
            }

            m_stack.back()->attributes[attrName] = std::move(attr);
        } else {
            throw Exception("Unsupported node type: " + std::string(name));
        }
    }

    return resource;
}

