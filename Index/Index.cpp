
#include "stdafx.h"
#include "PAKReader.h"

int main()
{
    try {
        PAKReader reader;
        reader.read(R"(C:\Users\triec\Desktop\Tom.pak)");
        reader.explode(R"(C:\Users\triec\Desktop\Tom_Unpacked_By_BG3ModMaker)");
        

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
