#include "Frontend.h"

// ToDo: TreeConstructor in main

int main (int argc, const char** argv)
{
    if (argc < 2)
    {
        printf ("Error: No file name input.\n");
        return 0;
    }

    Tree code = {};
    TreeConstructor (&code);

    if (GoTree (&code, argv[1]) == 0)
        SaveTree (&code);

    TreeDestructor (&code);
    return 0;
}

bool GoTree (Tree* tree, const char* file_path)
{
    assert (tree);
    assert (file_path);
    
    if (LexicalAnalyze (tree, file_path))
        return 1;

    if (LexicalParse (tree))
        return 1;

    return 0;
}

bool LexicalAnalyze (Tree* tree, const char* file_path)
{
    assert (tree);
    assert (file_path);

    char* code = nullptr;
    if (!ReadTxt (&code, file_path))
        return 1;

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
            printf ("Error in line %lu\n", line_now);
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
            TreeDestructor (tree);
            return 1;
        }
    }
    while (el_now->type != DEC && tree->stk.size < tree->stk.capacity);
    
    tree->head = tree->head->right;
    return 0;
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
    el_result->type    = PARAM;
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

    (*el_now)->type = VAR;
    el_result->type = EQUAL;

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
    el_result->type = CALL;
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

    // Old code:

    /* const size_t without_backslash_0 = 1;
    const size_t return_size  = sizeof (RETURN_STR) - without_backslash_0;

    if ((*el_now)->len != return_size + 2 * MALE_LEN || 
        strncmp (RETURN_STR, (*el_now)->ind + MALE_LEN, return_size))
        return nullptr; */

    if (strcmp (RETURN_STR, (*el_now)->ind))
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

    // Old code:

/*     const size_t without_backslash_0 = 1;

    const size_t    if_size  = sizeof (IF_STR)    - without_backslash_0;
    const size_t while_size  = sizeof (WHILE_STR) - without_backslash_0;

    if ((*el_now)->len != if_size    + 2 * MALE_LEN && 
        (*el_now)->len != while_size + 2 * MALE_LEN) 
        require_exit;

         if (strncmp (IF_STR,    (*el_now)->ind + MALE_LEN,    if_size) == 0)
        (*el_now)->type = IF;
    else if (strncmp (WHILE_STR, (*el_now)->ind + MALE_LEN, while_size) == 0)
        (*el_now)->type = WHILE;
    else
        require_exit;
 */

    if (strcmp (IF_STR, (*el_now)->ind) == 0)
        (*el_now)->type = IF;
    else if (strcmp (WHILE_STR, (*el_now)->ind) == 0)
        (*el_now)->type = WHILE;
    else
        require_exit;

    element* el_result = *el_now;
    next;

    el_result->left = GetE (el_now);
    check_parse (el_result->left);

    if ((*el_now)->type == ARITH && 
        (*el_now)->len  == 1     &&
      *((*el_now)->ind) != '>'   &&
      *((*el_now)->ind) != '<')
            require_exit;
            
    (*el_now)->left       = el_result->left;
    el_result->left       = *el_now;
    el_result->left->type = COND;
    next;

    el_result->left->right = GetE (el_now);
    check_parse (el_result->left->right);

    el_result->right = GetBody (el_now);
    check_parse (el_result->right);
    
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

        default:
            return PARSE_ERR;
    }

    return el_result;
}
