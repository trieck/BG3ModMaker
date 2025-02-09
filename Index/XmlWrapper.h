#pragma once

#include <pugixml.hpp>

class XmlWrapper
{
public:
    explicit XmlWrapper(const ByteBuffer& buffer);
    ~XmlWrapper();

    const pugi::xml_document& doc() const;

    std::string asString() const;
    pugi::xpath_node selectNode(const char* xpath) const;
    pugi::xpath_node_set selectNodes(const char* xpath) const;

private:
    void load(const ByteBuffer& buffer);

    pugi::xml_document m_doc;
};

