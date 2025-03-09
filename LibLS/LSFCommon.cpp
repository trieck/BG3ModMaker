#include "pch.h"

#include <pugixml.hpp>

#include "LSFCommon.h"

namespace { // anonymous namespace
    void addNodes(pugi::xml_node xmlNode, const std::vector<RBNode::Ptr>& nodes);

    void addNode(pugi::xml_node xmlNode, const RBNode::Ptr& node)
    {
        auto nodeNode = xmlNode.append_child("node");
        nodeNode.append_attribute("id") = node->name.c_str();
        for (const auto& [key, val] : node->attributes) {
            auto attributeNode = nodeNode.append_child("attribute");
            attributeNode.append_attribute("id") = key.c_str();
            attributeNode.append_attribute("type") = val.typeStr().c_str();
            attributeNode.append_attribute("value") = val.value().c_str();

        }

        if (node->childCount() > 0) {
            auto child = nodeNode.append_child("children");
            for (const auto& val : node->children | std::views::values) {
                addNodes(child, val);
            }
        }
    }

    void addNodes(pugi::xml_node xmlNode, const std::vector<RBNode::Ptr>& nodes)
    {
        for (const auto& node : nodes) {
            addNode(xmlNode, node);
        }
    }

    void addRegion(pugi::xml_node& xmlNode, const Region& region)
    {
        auto regionNode = xmlNode.append_child("region");
        regionNode.append_attribute("id") = region.regionName.c_str();

        auto childNode = regionNode.append_child("node");
        auto childAttr = childNode.append_attribute("id");
        childAttr.set_value("Templates");

        if (region.childCount() > 0) {
            auto child = childNode.append_child("children");
            for (const auto& val : region.children | std::views::values) {
                addNodes(child, val);
            }
        }
    }
}

XmlWrapper Resource::toXml() const
{
    using namespace pugi;

    xml_document doc;
    auto declarationNode = doc.append_child(node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "utf-8";

    auto root = doc.append_child("save");
    auto versionNode = root.append_child("version");
    versionNode.append_attribute("major") = metadata.majorVersion;
    versionNode.append_attribute("minor") = metadata.minorVersion;
    versionNode.append_attribute("revision") = metadata.revision;
    versionNode.append_attribute("build") = metadata.buildNumber;

    for (const auto& val : regions | std::views::values) {
        addRegion(root, *val);
    }

    return XmlWrapper(doc);
}
