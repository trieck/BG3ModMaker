#pragma once

#include <xapian.h>

class Searcher
{
public:
    Searcher() = default;
    ~Searcher() = default;

    static Xapian::MSet search(const char* dbName, const char* query);
};
