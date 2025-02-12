#include "pch.h"
#include "XmlWrapper.h"

XmlWrapper::XmlWrapper(const ByteBuffer& buffer)
{
    load(buffer);
}

XmlWrapper::~XmlWrapper()
= default;

const pugi::xml_document& XmlWrapper::doc() const
{
    return m_doc;
}

std::string XmlWrapper::asString() const
{
    std::ostringstream oss;
    m_doc.save(oss);
    return oss.str();
}

pugi::xpath_node XmlWrapper::selectNode(const char* xpath) const
{
    return m_doc.select_node(xpath);
}

pugi::xpath_node_set XmlWrapper::selectNodes(const char* xpath) const
{
    return m_doc.select_nodes(xpath);
}

void XmlWrapper::load(const ByteBuffer& buffer)
{
    // Convert buffer to string
    std::string xml(buffer.first.get(), buffer.first.get() + buffer.second);

    // Strip UTF-8 BOM if present
    static const std::string UTF8_BOM = "\xEF\xBB\xBF";
    if (xml.starts_with(UTF8_BOM)) {
        xml.erase(0, UTF8_BOM.size());
    }

    // Parse the XML document
    pugi::xml_parse_result result = m_doc.load_string(xml.c_str());
    if (!result) {
        throw std::runtime_error("Failed to parse XML document.");
    }
}
