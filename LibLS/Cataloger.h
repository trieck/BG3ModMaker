#pragma once

#include "Node.h"
#include "ObjectManager.h"
#include "PageableIterator.h"
#include "PAKReader.h"
#include "ProgressListener.h"
#include "Resource.h"

#include <nlohmann/json.hpp>
#include <rocksdb/db.h>


class Cataloger
{
public:
    Cataloger();
    virtual ~Cataloger();

    bool isOpen() const;
    nlohmann::json get(const std::string& key);
    PageableIterator::Ptr newIterator(const char* key, size_t pageSize = 25);
    PageableIterator::Ptr newIterator(size_t pageSize = 25);
    PrefixIterator::Ptr getChildren(const char* parent) const;
    PrefixIterator::Ptr getRoots() const;
    PrefixIterator::Ptr getRoots(const char* type) const;
    PrefixIterator::Ptr getTypes() const;
    void catalog(const char* pakFile, const char* dbName, bool overwrite = false);
    void close();
    void open(const char* dbName);
    void openReadOnly(const char* dbName);
    void setProgressListener(IFileProgressListener* listener);

private:
    void catalogLSXFile(const PackagedFileInfo& file);
    void catalogLSFFile(const PackagedFileInfo& file);
    void catalogNode(const std::string& filename, const LSNode::Ptr& node);
    void catalogNodes(const std::string& filename, const std::vector<LSNode::Ptr>& nodes);
    void catalogRegion(const std::string& fileName, const Region::Ptr& region);

    PAKReader m_reader;
    IFileProgressListener* m_listener = nullptr;
    ObjectManager m_objectManager;
};
