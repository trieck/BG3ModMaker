#include "stdafx.h"
#include "GR2Reader.h"
#include "Timer.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " {file-path}\n";
        return 1;
    }

    try {
        Timer timer;

        GR2Reader reader;
        reader.read(argv[1]);
        reader.traverse();

        std::cout << "   Reading took: " << timer.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
