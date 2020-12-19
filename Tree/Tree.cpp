#include "Stack.h"

/*
    Арифметическое переполнение: использование оператора "*" на байтовом значении 4 и
    приведение результата к байтовому значению 8. Приведите значение к более широкому
    типу перед вызовом оператора "*", чтобы избежать переполнения (io.2).
*/
#pragma warning (disable : 26451)

// Тип перечисления не входит в область. Старайтесь использовать "enum class" вместо "enum".
#pragma warning (disable : 26812)

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

bool LexicalAnalyze (Tree* tree, const char* file_path)
{
    assert (tree);
    assert (file_path);

    char* code = nullptr;
    if (!ReadTxt (&code, file_path))
        return 1;

    TreeConstructor (tree);
    size_t line_now = 1;
    char* code_now = code;
    SkipSpaces (&code_now, &line_now);

    while (*code_now != '\0')
    {
        if (tree->stk.status_error != STK_GOOD)
        {
            printf ("Stack error!\n");
            return 1;
        }

        if (GetElement (tree, &code_now))
        {
            printf ("Error in line %u\n", line_now);
            TreeDestructor (tree);
            return 1;
        }

        SkipSpaces (&code_now, &line_now);
    }

    free (code);
    return 0;
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

void  SkipSpaces (char** code, size_t* line_now)
{
    assert (code);
    assert (*code);

    while (**code == ' ' || **code == '\t' || **code == '\r' || **code == '\n')
    {
        if (**code == '\n')
            *line_now += 1;
        *code += 1;
    }

    return;
}

bool GetElement (Tree* tree, char** code)
{
    lex_ass;

    int lex_check = 0;

    #define TryGet(func) lex_check = func (tree, code); \
                         if (lex_check)                 \
                            return lex_check - 1;

    TryGet (CheckLR);
    TryGet (CheckBody);
    TryGet (CheckParam);
    TryGet (CheckArith);
    TryGet (CheckInd);
    TryGet (CheckNum);

    #undef TryGet
    return 1;
}

bool IsMale (const char* code)
{
    assert (code);
    unsigned const char* check = (unsigned const char*) code;

    return (check[0] == 226 && check[1] == 153 && check[2] == 130);
}

LexType CheckLR    (Tree* tree, char** code)
{
    lex_ass;

    if (**code == '$')
    {
        ElementConstructor (tree, LR, code, 1);
        return SUCCESS;
    }
    
    return NOT_THIS;
}

LexType CheckBody  (Tree* tree, char** code)
{
    lex_ass;

    if (strncmp ("AAAAH", *code, 5) == 0)
    {
        ElementConstructor (tree, BODY, code, 5);
        return SUCCESS;
    }

    return NOT_THIS;
}

LexType CheckParam (Tree* tree, char** code)
{
    lex_ass;

    if (strncmp ("ass", *code, 3) == 0)
    {
        ElementConstructor (tree, PARAM, code, 3);
        return SUCCESS;
    }

    return NOT_THIS;
}

LexType CheckArith (Tree* tree, char** code)
{
    lex_ass;

    switch (**code)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
            ElementConstructor (tree, ARITH, code, 1);
            return SUCCESS;

        case '=':
        case '>':
        case '<':
        {
            size_t strong = 1;
            if (*(*code + 1) == '=')
                strong = 2;
            ElementConstructor (tree, ARITH, code, strong);
            return SUCCESS;
        }
        case '!':
            if (*(*code + 1) == '=')
            {
                ElementConstructor (tree, ARITH, code, 2);
                return SUCCESS;
            }
    }

    return NOT_THIS;
}

LexType CheckInd   (Tree* tree, char** code)
{
    lex_ass;

    if (IsMale (*code))
    {
        size_t len = 3;

        while ((*code)[len] != '\0' && !IsMale (*code + len))
            len++;

        if ((*code)[len] == '\0')
            return ERROR;

        len += 3;
        ElementConstructor (tree, NIL, code, len);
        return SUCCESS;
    }

    return NOT_THIS;
}

LexType CheckNum   (Tree* tree, char** code)
{
    lex_ass;

    size_t num_len = 0;

    while ('0' <= (*code)[num_len] && (*code)[num_len] <= '9')
        num_len++;

    if ((*code)[num_len] == '.' && num_len != 0)
    {
        num_len++;
        while ('0' <= (*code)[num_len] && (*code)[num_len] <= '9')
            num_len++;
    }

    if (num_len > 0)
    {
        ElementConstructor (tree, NUM, code, num_len);
        return SUCCESS;
    }

    return NOT_THIS;
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
