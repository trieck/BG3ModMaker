#include "stdafx.h"
#include "CoInit.h"
#include "Iconizer.h"
#include "Timer.h"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " {package-path} {db-path} <overwrite: true/false>\n";
        return 1;
    }

    auto overwrite = false;

    if (argc >= 4) {
        std::string overwriteStr = argv[3];
        std::ranges::transform(overwriteStr, overwriteStr.begin(), tolower);
        overwrite = (overwriteStr == "true" || overwriteStr == "1" || overwriteStr == "yes");
    }

    try {
        Timer timer;
        COMInitializer coinit;

        Iconizer iconizer;
        iconizer.iconize(argv[1], argv[2], overwrite);

        std::cout << "   Iconizing took: " << timer.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
