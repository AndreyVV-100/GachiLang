#include "Tree.h"

const char* TYPES_STR[] = {"NIL", "DEC", "FUNC", "LR", "PARAM", "BODY", "ARITH", "NUM", 
                           "VAR", "IND", "RET", "IF", "WHILE", "CALL", "EQUAL", "COND"};

const int SIZEOF_TYPES = sizeof (TYPES_STR) / sizeof (*TYPES_STR);

element* PARSE_ERR = (element*) 1;

void TreeConstructor (Tree* tree)
{
    assert (tree);

    const size_t stk_size = 1000;
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

bool ElementConstructor (Tree* tree, Types type, char** ind, size_t len)
{
    char* str = nullptr;

    if (*ind)
    {
        str = (char*) calloc (len + 1, sizeof (*str));
        if (!str)
        {
            printf ("Memory error!\n");
            return 1;
        }

        strncpy (str, *ind, len);
        *ind += len;
    }
    else
        assert (!len);

    element push = { type, nullptr, nullptr, str, len };
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

    system ("dot -Tpng AllDumps/out.dot -o AllDumps/gr.png");
    // system ("start AllDumps/gr.png");

    return;
}

void ElementGraph (FILE* graph, element* el)
{
    assert (graph);
    assert (el);

    if (el->left > PARSE_ERR)
    {
        fprintf (graph, "\"Point: %p\\n Type: %d\\n %s\" -> "
                        "\"Point: %p\\n Type: %d\\n %s\";\n",
                el,             el->type,       el->ind,
                el->left, el->left->type, el->left->ind);

        ElementGraph (graph, el->left);
    }

    if (el->right > PARSE_ERR)
    {
        fprintf (graph, "\"Point: %p\\n Type: %d\\n %s\" -> "
                        "\"Point: %p\\n Type: %d\\n %s\";\n",
                        el,        el->type,        el->ind,
                 el->right, el->right->type, el->right->ind);

        ElementGraph (graph, el->right);
    }

    return;
}

int CountSize (FILE* file)
{
    fseek (file, 0, SEEK_END);
    size_t num_symbols = ftell (file);
    fseek (file, 0, SEEK_SET);

    return num_symbols;
}

size_t ReadTxt (char** text, const char* file_name)
{
    assert (text);
    assert (file_name);

    FILE* file = fopen (file_name, "r");
    if (file == nullptr)
    {
        printf ("[Input error] Unable to open file \"%s\"\n", file_name);
        return 0;
    }

    size_t num_symbols = CountSize (file);

    *text = (char*)calloc (num_symbols + 4, sizeof (**text));

    if (*text == nullptr)
    {
        printf ("[Error] Unable to allocate memory\n");
        return 0;
    }

    fread (*text, sizeof (**text), num_symbols, file);
    fclose (file);

    (*text)[num_symbols] = '\0';

    return num_symbols;
}

void SaveTree (Tree* tree)
{
    assert (tree);

    FILE* tree_file = fopen ("CompileResult/CreatedTree.txt", "w");
    if (!tree_file)
    {
        printf ("Save error: Unable to open file \"CompileResult/CreatedTree.txt\"\n");
        return;
    }

    SaveElem (tree->head, tree_file, 0);
    fclose (tree_file);
    return;
}

void SaveElem (element* el, FILE* tree_file, int tab)
{
    assert (tree_file);

    if (!el)
    {
        fprintf (tree_file, "\n");
        return;
    }

    fprintf (tree_file, "%*s%s %s\n", tab * 4, "", TYPES_STR[el->type], el->ind);

    #define next(direction) fprintf (tree_file, "%*s{\n", tab * 4, "");   \
                            SaveElem (el->direction, tree_file, tab + 1); \
                            fprintf (tree_file, "%*s}\n", tab * 4, "")   

    next (left);
    next (right);
    #undef next
    return;
}

bool LoadTree (Tree* tree)
{
    assert (tree);
    
    if (tree->head)
    {
        printf ("Load error: Tree maybe is already created.\n");
        return 0;
    }

    char* tree_file = nullptr;
    ReadTxt (&tree_file, "CompileResult/CreatedTree.txt");
    char* pos_now = tree_file;
    tree->head = LoadElem (tree, &pos_now);
    
    free (tree_file);
    return tree->head != PARSE_ERR;
}

element* LoadElem (Tree* tree, char** pos_now)
{
    assert (tree);
    assert (pos_now);

    char* newline = strchr (*pos_now, '\n');
    if (!newline)
    {
        printf ("Bad tree-input format: no end of line.\n");
        return PARSE_ERR;
    }
    *newline = '\0';

    while (**pos_now == ' ')
        *pos_now += 1;

    if (**pos_now == '\0')
    {
        *pos_now += 1;
        return nullptr;
    }

    char* space = strchr (*pos_now, ' ');
    if (!space)
    {
        printf ("Bad tree-input format: no space after type.\n");
        return PARSE_ERR;
    }
    *space = '\0';

    int i_type = NIL;
    for (i_type = NIL; i_type < SIZEOF_TYPES; i_type++)
        if (strcmp (TYPES_STR[i_type], *pos_now) == 0)
            break;

    if (i_type == SIZEOF_TYPES)
    {
        printf ("Bad tree-input format: unknown type.\n");
        return PARSE_ERR;
    }

    element* elem_now = tree->stk.buffer + tree->stk.size;
    size_t ind_len = newline - space - 1;
    StackPush (&(tree->stk), {(Types) i_type, nullptr, nullptr, space + 1, ind_len, '\0'});

    *pos_now = newline + 1;
    #define skip_line newline = strchr (*pos_now, '\n');                        \
                      if (!newline)                                             \
                      {                                                         \
                          printf ("Bad tree-input format: no end of line.\n");  \
                          return PARSE_ERR;                                     \
                      }                                                         \
                      *pos_now = newline + 1

    skip_line;
    elem_now->left  = LoadElem (tree, pos_now);
    skip_line;
    skip_line;
    elem_now->right = LoadElem (tree, pos_now);
    skip_line;

    return (elem_now->left == PARSE_ERR || elem_now->right == PARSE_ERR) ? PARSE_ERR : elem_now;
}
