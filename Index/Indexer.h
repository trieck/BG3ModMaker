#pragma once
#include "PAKReader.h"

class Indexer
{
public:
    Indexer();
    virtual ~Indexer() = default;
    void index(const char* pakFile, const char* dbName);

private:
    PAKReader m_reader;
};

