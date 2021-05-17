#include "Backend/Backend.h"

int main (int argc, char** argv)
{
    // Frontend:
    // ToDo:
    // system ("Frontend.out");
    // ...
    // Or makefile
    Tree code = {};

    if (GoTree (&code, (argc > 1) ? argv[1] : "Examples/SqrtTripleDick.gcm"))
    {
        TreeDestructor (&code);
        return 0;
    }
    // CreateGraph (&code);

    // Backend:
    GoAsm  (&code);

    TreeDestructor (&code);
    return 0;
}
