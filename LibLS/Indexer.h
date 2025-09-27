#pragma once

#include <xapian.h>

#include "Node.h"
#include "PAKReader.h"
#include "Resource.h"

class IIndexProgressListener {
public:
    virtual ~IIndexProgressListener() = default;

    virtual void onStart(std::size_t totalEntries) = 0;
    virtual void onFileIndexing(std::size_t currentIndex, const std::string& filename) = 0;
    virtual void onFinished(std::size_t indexedEntries) = 0;
    virtual bool isCancelled() = 0;
    virtual void onCancel() = 0;
};

class Indexer
{
public:
    Indexer();
    virtual ~Indexer() = default;

    void index(const char* pakFile, const char* dbName, bool overwrite = false);
    void compact() const;
    void setProgressListener(IIndexProgressListener* listener);

private:
    void indexLSFFile(const PackagedFileInfo& file);
    void indexLSXFile(const PackagedFileInfo& file);
    void indexNode(const std::string& filename, const LSNode::Ptr& node);
    void indexNodes(const std::string& filename, const std::vector<LSNode::Ptr>& nodes);
    void indexRegion(const std::string& fileName, const Region::Ptr &region);
    void indexTXTFile(const PackagedFileInfo& file);

    using WritableDBPtr = std::unique_ptr<Xapian::WritableDatabase>;
    WritableDBPtr m_db;

    Xapian::TermGenerator m_termgen;
    Xapian::SimpleStopper m_stopper;
    PAKReader m_reader;
    IIndexProgressListener* m_listener = nullptr;
};

