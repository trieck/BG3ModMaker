#pragma once

#include "Node.h"
#include "PAKReader.h"
#include "Resource.h"

#include <rocksdb/db.h>

class ICatalogProgressListener
{
public:
    virtual ~ICatalogProgressListener() = default;

    virtual void onStart(std::size_t totalEntries) = 0;
    virtual void onFileCataloging(std::size_t currentcatalog, const std::string& filename) = 0;
    virtual void onFinished(std::size_t catalogedEntries) = 0;
    virtual bool isCancelled() = 0;
    virtual void onCancel() = 0;
};

class Cataloger
{
public:
    Cataloger();
    virtual ~Cataloger();

    void catalog(const char* pakFile, const char* dbName, bool overwrite = false);
    void setProgressListener(ICatalogProgressListener* listener);

private:
    void catalogLSXFile(const PackagedFileInfo& file);
    void catalogLSFFile(const PackagedFileInfo& file);
    void catalogNode(const std::string& filename, const LSNode::Ptr& node);
    void catalogNodes(const std::string& filename, const std::vector<LSNode::Ptr>& nodes);
    void catalogRegion(const std::string& fileName, const Region::Ptr& region);
    void close();

    PAKReader m_reader;
    ICatalogProgressListener* m_listener = nullptr;
    rocksdb::DB* m_db = nullptr;
};
