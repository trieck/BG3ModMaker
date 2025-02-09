
#include "stdafx.h"
#include "Indexer.h"

int main()
{
    try {
        Indexer indexer;
        indexer.index(R"(C:\Users\triec\Desktop\Tom.pak)");

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
