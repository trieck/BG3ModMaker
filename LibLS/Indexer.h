#pragma once
#include "LSFCommon.h"
#include "PAKReader.h"

class Indexer
{
public:
    Indexer();
    virtual ~Indexer() = default;

    void index(const char* pakFile, const char* dbName);
    void compact() const;

private:
    void indexLSXFile(const PackagedFileInfo& file);
    void indexNode(const std::string& filename, const Node::Ptr& node);
    void indexNodes(const std::string& filename, const std::vector<Node::Ptr>& nodes);
    void indexRegion(const std::string& fileName, const Region::Ptr &region);
    void indexLSFFile(const PackagedFileInfo& file);

    using WritableDBPtr = std::unique_ptr<Xapian::WritableDatabase>;
    WritableDBPtr m_db;

    Xapian::TermGenerator m_termgen;
    Xapian::SimpleStopper m_stopper;
    PAKReader m_reader;
};

