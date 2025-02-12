
#include "stdafx.h"
#include <nlohmann/json.hpp>
#include <iostream>

#include "Searcher.h"
#include "Timer.h"
#include "Utility.h"

using json = nlohmann::json;

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "usage: search.exe {db-path} {query}" << std::endl;
        return 1;
    }

    try {
        Timer timer;

        auto results = Searcher::search(argv[1], argv[2]);
        for (auto it = results.begin(); it != results.end(); ++it) {
            auto doc = json::parse(it.get_document().get_data());
            std::cout << doc.dump(4) << std::endl;
        }

        std::cout << "   Found: " << comma(results.size()) << " results" << std::endl;
        std::cout << "   Search took: " << timer.str() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const Xapian::Error& e) {
        std::cerr << "Error: " << e.get_description() << std::endl;
        return 1;
    }

    return 0;
}
