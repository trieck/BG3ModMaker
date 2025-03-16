#include "framework.h"
#include "Rope.h"

int main(int argc, char* argv[])
{
    Rope rope;
;
    rope.insert(0, "World");
    rope.insert(0, "Hello ");

    rope.exportDOT("d:\\tmp\\rope.dot");

    return 0;
}
