#include "stdafx.h"

#include <xapian.h>

#include "Indexer.h"
#include "Timer.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " {package-path} {db-path}\n";
        return 1;
    }

    std::string pakPath = argv[1];
    std::string dbPath = argv[2];

    try {
        Timer timer;

        Indexer indexer;
        indexer.index(argv[1], argv[2]);

        std::cout << "   Indexing took: " << timer.str() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const Xapian::Error& e) {
        std::cerr << "Error: " << e.get_description() << std::endl;
        return 1;
    }
    
    return 0;
}
