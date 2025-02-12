#include "pch.h"

#include "Searcher.h"

Xapian::MSet Searcher::search(const char* dbName, const char* query)
{
    Xapian::Database db(dbName);
    Xapian::Enquire enquire(db);

    Xapian::QueryParser parser;
    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_OR);

    auto xquery = parser.parse_query(query);
    enquire.set_query(xquery);

    return enquire.get_mset(0, db.get_doccount());
}
