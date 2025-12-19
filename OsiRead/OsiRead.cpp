#include "stdafx.h"

#include "OsiReader.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " {file-path}\n";
        return 1;
    }

    try {
        OsiReader reader;
        reader.readFile(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
