#pragma once

#include "Node.h"
#include "PAKReader.h"
#include "PageableIterator.h"
#include "ProgressListener.h"
#include "Resource.h"

#include <nlohmann/json.hpp>
#include <rocksdb/db.h>

class Cataloger
{
public:
    Cataloger();
    virtual ~Cataloger();

    void close();
    bool isOpen() const;
    PageableIterator::Ptr newIterator(size_t pageSize = 25);
    PageableIterator::Ptr newIterator(const char* key, size_t pageSize = 25);
    void catalog(const char* pakFile, const char* dbName, bool overwrite = false);
    nlohmann::json get(const std::string& key);
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
    rocksdb::DB* m_db = nullptr;
    rocksdb::WriteBatch m_batch;
};
