#include <iostream>

#include "PAKReader.h"

int main()
{
    PAKReader reader;

    try {
        reader.read(R"(C:\Program Files (x86)\Steam\steamapps\common\Baldurs Gate 3\Data\Gustav.pak)");
        reader.explode("C:/Users/triec/Desktop/BG3ModMaker");
        

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
