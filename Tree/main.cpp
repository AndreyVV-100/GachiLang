#include "Tree.h"
#include <windows.h>

int main ()
{
    SetConsoleCP (65001);
    SetConsoleOutputCP (65001);

    Tree tree = {};
    LexicalAnalyze (&tree, "../Examples/pr1.txt");
    element* elems = tree.stk.buffer;

    for (int i_elem = 0; i_elem < 100; i_elem++)
        printf ("Type: %d - %s\n", elems[i_elem].type, (elems[i_elem].ind) ? elems[i_elem].ind : "");

    TreeDestructor (&tree);

    return 0;
}
