#include "Stack.h"

#pragma warning (disable : 26451)
/*
    Арифметическое переполнение: использование оператора "*" на байтовом значении 4 и
    приведение результата к байтовому значению 8. Приведите значение к более широкому
    типу перед вызовом оператора "*", чтобы избежать переполнения (io.2).
*/

void TreeConstructor (Tree* tree)
{
    assert (tree);

    const size_t stk_size = 100;
    tree->head = nullptr;
    StackConstructor (&(tree->stk), stk_size);
    return;
}

void TreeDestructor (Tree* tree)
{
    assert (tree);

    StackDestructor (&(tree->stk));
    tree->head = nullptr;

    return;
}

bool ElementConstructor (Tree* tree, Types type, char* ind, size_t num)
{
    char* str = (char*) calloc (num + 1, sizeof (*str));
    if (!str)
    {
        printf ("Memory error!\n");
        return 1;
    }

    strncpy (str, ind, num);
    element push = { type, nullptr, nullptr, str, num };
    StackPush (&(tree->stk), push);

    if (tree->stk.status_error != STK_GOOD)
        return 1;

    return 0;
}

void ElementDestructor (element* el)
{
    assert (el);

    free (el->ind);
    *el = STK_POISON;
    return;
}

void CreateGraph (Tree* tree)
{
    assert (tree);

    FILE* graph = fopen ("AllDumps/out.dot", "w");
    assert (graph);
    fprintf (graph, "digraph G{\n" "rankdir = HR;\n node[shape=box];\n");

    if (tree->head)
    {
        fprintf (graph, "\"Point: %p\\n Type: %d\\n %s\";\n",
            tree->head, tree->head->type, tree->head->ind);
        ElementGraph (graph, tree->head);
    }
    else
        fprintf (graph, "\"No elements\";\n");

    fprintf (graph, "}");
    fclose (graph);

    system ("dot -Tpng AllDumps\\out.dot -o AllDumps\\gr.png");
    system ("start AllDumps\\gr.png");

    return;
}

void ElementGraph (FILE* graph, element* el)
{
    assert (graph);
    assert (el);

    if (el->left)
    {
        fprintf (graph, "\"Point: %p\\n Type: %d\\n %s\" -> "
                        "\"Point: %p\\n Type: %d\\n %s\";\n",
                el,             el->type,       el->ind,
                el->left, el->left->type, el->left->ind);

        ElementGraph (graph, el->left);
    }

    if (el->right)
    {
        fprintf (graph, "\"Point: %p\\n Type: %d\\n %s\" -> "
                        "\"Point: %p\\n Type: %d\\n %s\";\n",
                        el,        el->type,        el->ind,
                 el->right, el->right->type, el->right->ind);

        ElementGraph (graph, el->right);
    }

    return;
}

/*
Old and uselessful code:

element* InsertHead   (Tree* tree, Types type, double num, char symb)
{
    assert (tree);
    assert (tree->head == nullptr);

    create_el;
    tree->head = el_crt;

    return el_crt;
}

element* InsertLeft  (element* el, Types type, double num, char symb)
{
    assert (el->left);

    create_el;
    el->left = el_crt;

    return el_crt;
}

element* InsertRight (element* el, Types type, double num, char symb)
{
    assert (el->right);

    create_el;
    el->right = el_crt;

    return el_crt;
}
*/
