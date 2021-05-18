#include "Tree.h"

int main ()
{
    Tree tree = {};
    GoTree (&tree, "../Examples/pr1.txt");

    // CreateGraph  (&tree); 
    
    element* elems = tree.stk.buffer;

    for (int i_elem = 0; i_elem < 100; i_elem++)
        printf ("Type: %d - %s\n", elems[i_elem].type, (elems[i_elem].ind) ? elems[i_elem].ind : "");
    

    TreeDestructor (&tree);

    return 0;
}
