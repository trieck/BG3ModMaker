#include "stdafx.h"
#include "Indexer.h"
#include "Timer.h"

#include <xapian.h>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " {package-path} {index-path} <overwrite: true/false>\n";
        return 1;
    }

    std::string pakPath = argv[1];
    std::string dbPath = argv[2];

    auto overwrite = false;

    if (argc >= 4) {
        std::string overwriteStr = argv[3];
        std::ranges::transform(overwriteStr, overwriteStr.begin(), tolower);
        overwrite = (overwriteStr == "true" || overwriteStr == "1" || overwriteStr == "yes");
    }

    try {
        Timer timer;

        Indexer indexer;
        indexer.index(argv[1], argv[2], overwrite);

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
