#pragma once
#include "PAKReader.h"

class Indexer
{
public:
    Indexer();
    virtual ~Indexer() = default;

    void index(const char* pakFile, const char* dbName);
    void compact() const;

private:
    void indexLSXFile(const PackagedFileInfo& file) const;
    void indexLSFFile(const PackagedFileInfo& file) const;

    using WritableDBPtr = std::unique_ptr<Xapian::WritableDatabase>;
    WritableDBPtr m_db;

    Xapian::SimpleStopper m_stopper;
    PAKReader m_reader;
};

