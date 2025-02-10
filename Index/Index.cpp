
#include "stdafx.h"

#include <nlohmann/json.hpp>

#pragma warning(push)
#pragma warning(disable : 4996)  // Suppress deprecation warnings
#include <xapian.h>
#pragma warning(pop)

#include "Indexer.h"

using json = nlohmann::json;

void searchdb(const char* dbName)
{
    // Try retrieving an indexed GameObject
    std::string uuid = "2f5636ff-8f50-47cd-9e3e-267ee39e806b"; // Example MapKey
    std::string result;

    Xapian::Database db(dbName);

    Xapian::Enquire enquire(db);

    Xapian::QueryParser parser;
    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_OR);
    Xapian::Query query = parser.parse_query(uuid);

    enquire.set_query(query);

    Xapian::MSet matches = enquire.get_mset(0, 10);

    std::cout << "Search results for: " << uuid << "\n";
    for (auto it = matches.begin(); it != matches.end(); ++it) {
        Xapian::Document doc = it.get_document();
        std::cout << "Match found: " << doc.get_data() << "\n";
    }
}

int main()
{
    try {
        Indexer indexer;
        indexer.index(R"(C:\Users\triec\Desktop\Tom.pak)", R"(C:\Users\triec\Desktop\bg3_db)");

        searchdb(R"(C:\Users\triec\Desktop\bg3_db)");

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
