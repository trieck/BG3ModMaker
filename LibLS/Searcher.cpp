#include "pch.h"

#include "Searcher.h"

Xapian::MSet Searcher::search(const char* dbName, const char* query, uint32_t offset, uint32_t pageSize)
{
    Xapian::Database db(dbName);
    Xapian::Enquire enquire(db);
    Xapian::QueryParser parser;

    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_AND);

    auto xquery = parser.parse_query(query);
    enquire.set_query(xquery);

    return enquire.get_mset(offset, pageSize);
}
