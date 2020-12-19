#include "Stack.h"

/*
    Арифметическое переполнение: использование оператора "*" на байтовом значении 4 и
    приведение результата к байтовому значению 8. Приведите значение к более широкому
    типу перед вызовом оператора "*", чтобы избежать переполнения (io.2).
*/
#pragma warning (disable : 26451)

// Тип перечисления не входит в область. Старайтесь использовать "enum class" вместо "enum".
#pragma warning (disable : 26812)

element* PARSE_ERR = (element*) 1;

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

        if (CheckElement (tree, &code_now))
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

bool LexicalParse (Tree* tree)
{
    assert (tree);

    element* el_now  = tree->stk.buffer;
    const char dec_str[] = "DICK";
    char* fix_constructor = (char*) dec_str;

    ElementConstructor (tree, DEC, (char**) &fix_constructor, 4);
    fix_constructor -= 4;

    element* dec_now = tree->stk.buffer + tree->stk.size - 1;
    tree->head = dec_now;

    // ToDo: Think about going out of bounds.

    do
    {
        ElementConstructor (tree, DEC, &fix_constructor, 4);
        fix_constructor -= 4;

        dec_now->right = tree->stk.buffer + tree->stk.size - 1;
        dec_now = dec_now->right;

        dec_now->left = GetFunc (&el_now);
        if (dec_now->left == PARSE_ERR)
        {
            //TreeDestructor (tree);
            return 1;
        }
    }
    while (el_now->type != DEC && tree->stk.size < tree->stk.capacity);
    
    tree->head = tree->head->right;
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

bool CheckElement (Tree* tree, char** code)
{
    anal_ass;

    int lex_check = 0;

    #define TryCheck(func) lex_check = func (tree, code); \
                           if (lex_check)                 \
                              return lex_check - 1;

    TryCheck (CheckLR);
    TryCheck (CheckBody);
    TryCheck (CheckParam);
    TryCheck (CheckArith);
    TryCheck (CheckInd);
    TryCheck (CheckNum);

    #undef TryCheck
    return 1;
}

bool IsMale (const char* code)
{
    assert (code);
    unsigned const char* check = (unsigned const char*) code;

    return (check[0] == 226 && check[1] == 153 && check[2] == 130);
}

LexResult CheckLR    (Tree* tree, char** code)
{
    anal_ass;

    if (**code == '$')
    {
        ElementConstructor (tree, LR, code, 1);
        return SUCCESS;
    }
    
    return NOT_THIS;
}

LexResult CheckBody  (Tree* tree, char** code)
{
    anal_ass;

    if (strncmp ("AAAAH", *code, 5) == 0)
    {
        ElementConstructor (tree, BODY, code, 5);
        return SUCCESS;
    }

    return NOT_THIS;
}

LexResult CheckParam (Tree* tree, char** code)
{
    anal_ass;

    if (strncmp ("ass", *code, 3) == 0)
    {
        ElementConstructor (tree, PARAM, code, 3);
        return SUCCESS;
    }

    return NOT_THIS;
}

LexResult CheckArith (Tree* tree, char** code)
{
    anal_ass;

    switch (**code)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '(':
        case ')':
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

LexResult CheckInd   (Tree* tree, char** code)
{
    anal_ass;

    if (IsMale (*code))
    {
        size_t len = 3;

        while ((*code)[len] != '\0' && !IsMale (*code + len))
            len++;

        if ((*code)[len] == '\0')
            return ERROR;

        len += 3;
        ElementConstructor (tree, IND, code, len);
        return SUCCESS;
    }

    return NOT_THIS;
}

LexResult CheckNum   (Tree* tree, char** code)
{
    anal_ass;

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

element* GetFunc      (element** el_now)
{
    parse_ass;

    require_ind;

    element* el_result = *el_now;
    next;

    el_result->left = GetFuncParam (el_now);
    check_parse (el_result->left);

    el_result->right = GetBody (el_now);
    check_parse (el_result->right);

    el_result->type = FUNC;
    return el_result;
}

element* GetFuncParam (element** el_now)
{
    parse_ass;

    require (PARAM);
    next;

    if ((*el_now)->type == PARAM)
    {
        next;
        return nullptr;
    }

    require_ind;
    element* el_result = *el_now;
    next;

    while ((*el_now)->type != PARAM)
    {
        require_ind;
        (*el_now)->type = PARAM;
        ((*el_now) - 1)->left = *el_now;
        next;
    }
    
    next;
    return el_result;
}

element* GetBody      (element** el_now)
{
    parse_ass;

    if ((*el_now)->type != BODY)
        return GetOper (el_now);
    next;
    
    if ((*el_now)->type == BODY)
    {
        next;
        return nullptr;
    }

    element* el_result = GetOper (el_now);
    check_parse (el_result);
    element* go_LR     = el_result;

    while ((*el_now)->type != BODY)
    {
        go_LR->right = GetOper (el_now);
        go_LR = go_LR->right;
        check_parse (go_LR);
    }

    require (BODY);
    next;
    return el_result;
}

element* GetOper      (element** el_now)
{
    parse_ass;
    require_ind;

    element* el_result = nullptr;

    #define TryGet(func) el_result = func (el_now);         \
                         if (el_result)                     \
                         {                                  \
                             check_parse (el_result);       \
                             require (LR);                  \
                             (*el_now)->left = el_result;   \
                             el_result = *el_now;           \
                             next;                          \
                             return el_result;              \
                         }

    TryGet (GetArith);
    TryGet (GetCall);
    TryGet (GetReturn);

    #undef TryGet
    return GetCond (el_now);
}

element* GetArith     (element** el_now)
{
    parse_ass;
    require_ind;

    if ((*el_now + 1)->type != ARITH)
        return nullptr;

    if (strcmp ("=", (*el_now + 1)->ind))
        require_exit;

    element* el_result = *el_now + 1;
    el_result->left    = *el_now;
    (*el_now)->type    = VAR;
    next; next;
    el_result->right   = GetE (el_now);

    check_parse (el_result->right);
    return el_result;
}

element* GetCall      (element** el_now)
{
    parse_ass;
    require_ind;

    if ((*el_now + 1)->type != PARAM)
        return nullptr;

    element* el_result = *el_now;
    next;

    el_result->left = GetCallParam (el_now);
    check_parse (el_result->left);
    el_result->type = FUNC;
    return el_result;
}

element* GetCallParam (element** el_now)
{
    parse_ass;

    require (PARAM);
    next;

    if ((*el_now)->type == PARAM)
    {
        next;
        return nullptr;
    }

    if ((*el_now)->type != NUM)
    {
        require_ind;
        (*el_now)->type = VAR;
    }

    element* el_result = *el_now;
    next;

    while ((*el_now)->type != PARAM)
    {
        if ((*el_now)->type != NUM)
        {
            require_ind;
            (*el_now)->type = VAR;
        }

        ((*el_now) - 1)->left = *el_now;
        next;
    }

    next;
    return el_result;
}

element* GetReturn    (element** el_now)
{
    parse_ass;
    require_ind;

    const char   return_str[] = "next door";
    const size_t return_size  = sizeof (return_str) + 5; // With attention to male symbol

    if ((*el_now)->len != return_size || strncmp (return_str, (*el_now)->ind + 3, (*el_now)->len - 6)) // +3 and -6 - skip male symbol
        return nullptr;

    element* el_result = *el_now;
    next;

    el_result->left = GetE (el_now);
    check_parse (el_result->left);
    el_result->type = RET;
    return el_result;
}

element* GetCond      (element** el_now)
{
    parse_ass;
    require_ind;

    const char   if_str[] = "fantasies";
    const size_t if_size  = sizeof (if_str) + 5; // With attention to male symbol
    const char   while_str[] = "let's go";
    const size_t while_size  = sizeof (while_str) + 5; // With attention to male symbol

    if ((*el_now)->len != if_size && (*el_now)->len != while_size) // +3 and -6 - skip male symbol
        return nullptr;

    if (strncmp (if_str, (*el_now)->ind + 3, (*el_now)->len - 6) && strncmp (while_str, (*el_now)->ind + 3, (*el_now)->len - 6))
        return nullptr;

    element* el_result = *el_now;
    next;

    el_result->left = GetE (el_now);
    check_parse (el_result->left);

    if ((*el_now)->type == ARITH && 
        (*el_now)->len  == 1     &&
      *((*el_now)->ind) != '>'   &&
      *((*el_now)->ind) != '<')
            require_exit;
            
    (*el_now)->left = el_result->left;
    el_result->left = *el_now;
    next;

    el_result->left->right = GetE (el_now);
    check_parse (el_result->left->right);

    el_result->right = GetBody (el_now);
    check_parse (el_result->right);
    el_result->type = COND;
    
    // Gypsy focuses
    if ((*el_now - 1)->type == LR)
    {
        // *el_now - 1 == el_result->right
        el_result->right    = el_result->right->left;
    }

    else
    {
        // (*el_now - 1)->type == BODY - useless element, I can use it for good deed.
        (*el_now - 1)->type = LR;
        strcpy((*el_now - 1)->ind, "$");    
    }

    (*el_now - 1)->left = el_result;
    el_result = *el_now - 1;
    // End of gypsy focuses
    return el_result;
}

element* GetE         (element** el_now)
{
    parse_ass;

    element* el_result = GetT (el_now);

    while ((*el_now)->type == ARITH           && 
          (strcmp ("+", (*el_now)->ind) == 0  ||
           strcmp ("-", (*el_now)->ind) == 0))
    {
        (*el_now)->left = el_result;
        el_result = *el_now;
        next;
        el_result->right = GetT (el_now);
        check_parse (el_result->right);
    }

    return el_result;
}

element* GetT         (element** el_now)
{
    parse_ass;

    element* el_result = GetDegree (el_now);

    while ((*el_now)->type == ARITH &&
          (strcmp ("*", (*el_now)->ind) == 0 ||
           strcmp ("/", (*el_now)->ind) == 0))
    {
        (*el_now)->left = el_result;
        el_result = *el_now;
        next;
        el_result->right = GetDegree (el_now);
        check_parse (el_result->right);
    }

    return el_result;
}

element* GetDegree    (element** el_now)
{
    parse_ass;

    element* el_result = GetUnary (el_now);

    if ((*el_now)->type == ARITH && strcmp ("^", (*el_now)->ind) == 0)
    {
        (*el_now)->left = el_result;
        el_result = *el_now;
        next;
        el_result->right = GetDegree (el_now);
        check_parse (el_result->right);
    }

    return el_result;
}

element* GetUnary     (element** el_now)
{
    parse_ass;

    if ((*el_now + 1)->type == PARAM)
    {
        element* el_result = GetCall (el_now);
        if (!el_result)
            require_exit;
        return el_result;
    }

    return GetP (el_now);
}

element* GetP         (element** el_now)
{
    parse_ass;

    element* el_result = PARSE_ERR;

    switch ((*el_now)->type)
    {
        case ARITH:
            if (strcmp ("(", (*el_now)->ind))
                require_exit;

            next;
            el_result = GetE (el_now);

            if (strcmp (")", (*el_now)->ind))
                require_exit;
            next;
            break;

        case IND:
            el_result = *el_now;
            next;
            el_result->type = VAR;
            break;

        case NUM:
            el_result = *el_now;
            next;
            break;
    }

    return el_result;
}
