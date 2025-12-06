#include "pch.h"

#include "Searcher.h"

Xapian::MSet Searcher::search(const char* dbName, const char* query, uint32_t offset, uint32_t pageSize)
{
    return search(dbName, Xapian::Query(query), offset, pageSize);
}

Xapian::MSet Searcher::search(const char* dbName, const Xapian::Query& query, uint32_t offset, uint32_t pageSize)
{
    Xapian::Database db(dbName);
    Xapian::Enquire enquire(db);
    Xapian::QueryParser parser;

    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_AND);
    enquire.set_query(query);

    return enquire.get_mset(offset, pageSize);
}
