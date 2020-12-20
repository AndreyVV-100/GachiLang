#include "Backend.h"

int main ()
{
    Tree code = {};

    if (GoTree (&code, "../Examples/SqrtTripleDick.txt"))
    {
        TreeDestructor (&code);
        return 0;
    }
    CreateGraph (&code);
    GoAsm  (&code, "AsmCode/asm.txt");

    TreeDestructor (&code);
    return 0;
}

void GoAsm (Tree* code, const char* file_path)
{
    assert (code);
    assert (file_path);

    FILE* asm_text = fopen (file_path, "w");
    if (!asm_text)
    {
        printf ("Error: Unable to write file %s\n", file_path);
        return;
    }

    if (PrintService (asm_text, "ServiceFiles/AsmBegin.txt"))
    {
        fclose (asm_text);
        return;
    }

    DeleteFuncSpaces (code->head);
    
    PrintDec (asm_text, code->head);

    fclose (asm_text);
    return;
}

bool PrintService (FILE* asm_text, const char* file_path)
{
    assert (asm_text);

    char* asm_begin = nullptr;
    
    if (!ReadTxt (&asm_begin, file_path))
        return 1;

    fprintf (asm_text, "%s", asm_begin);
    free (asm_begin);
}

void DeleteFuncSpaces (element* el)
{
    if (!el || (el->type != FUNC && el->type != CALL))
        return;

    char* ind = el->ind;

    while (*ind != '\0')
    {
        if (*ind == ' ' || *ind == '\t' || *ind == '\r' || *ind == '\n')
            *ind = '_';
        ind++;
    }

    DeleteFuncSpaces (el->left);
    DeleteFuncSpaces (el->right);

    return;
}

bool PrintDec (FILE* asm_text, element* el)
{
    assert (asm_text);
    assert (el);
    
    if (el->type != DEC)
        return 1;

    if (PrintFunc (asm_text, el->left))
        return 1;

    if (el->right && PrintDec (asm_text, el->right))
        return 1;

    return 0;
}

bool PrintFunc (FILE* asm_text, element* el)
{
    assert (asm_text);
    assert (el);

    if (el->type != FUNC)
        return 1;

    fprintf (asm_text, "\n""%s:\n", el->ind);

    const size_t max_var_num = 32;
    Stack vars_ = {};
    StackConstructor (&vars_, max_var_num);
    if (TakeFuncVars (el, &vars_))
    {
        StackDestructor (&vars_);
        return 1;
    }

    Stack* vars = &vars_;
    TryPrint (PrintParam, left);
    TryPrint (PrintLR,   right);

    return 0;
}

bool TakeFuncVars (element* el, Stack* vars)
{
    assert (vars);
    if (!el)
        return 0;

    if (el->type == PARAM)
    {
        if (el->left && el->left->type != PARAM)
        {
            printf ("TakeFuncVars error: bad parameters.\n");
            return 1;
        }

        StackPush (vars, *el);

        if (el->left && TakeFuncVars (el->left, vars))
            return 1;

        return 0;
    }

    if (el->type == EQUAL)
    {
        if (!(el->left) || el->left->type != VAR)
        {
            printf ("TakeFuncVars error: bad equality.\n");
            return 1;
        }

        if (VarNumber (vars, el->left->ind))
            return 0;

        StackPush (vars, *(el->left));
        return 0;
    }

    if (TakeFuncVars (el->left, vars))
        return 1;
    return TakeFuncVars (el->right, vars);
}

bool PrintParam (FILE* asm_text, element* el, Stack* vars)
{
    assert (asm_text);
    assert (vars);

    if (PrintService (asm_text, "ServiceFiles/FuncBegin.txt"))
        return 1;
    fprintf (asm_text, "\n""push %d\n"
                           "pop [rax + 1]\n",
             vars->size + 2);

    if (SaveParam (asm_text, el, vars))
        return 1;

    return 0;
}

bool SaveParam (FILE* asm_text, element* el, Stack* vars)
{
    assert (asm_text);
    assert (vars);
    if (!el)
        return 0;

    TryPrint (SaveParam, left);
    size_t var_num = VarNumber (vars, el->ind);
    if (var_num < 2)
        return 1;
    fprintf (asm_text, "pop [rax + %u]\n", var_num);
    return 0;
}

size_t VarNumber (Stack* vars, const char* var_name)
{
    assert (vars);
    assert (var_name);

    for (size_t result = 0; result < vars->size; result++)
        if (strcmp (var_name, vars->buffer[result].ind) == 0)
            return result + 2;

    // printf ("Error: No var with name \"%s\".\n", var_name);
    return 0;
}

bool PrintLR    (FILE* asm_text, element* el, Stack* vars)
{
    print_ass;

    if (el->type == LR)
    {
        if (el->left)
            TryPrint (PrintLR, left);
        if (el->right)
            TryPrint (PrintLR, right);
        return 0;
    }

    switch (el->type)
    {
        case EQUAL:
            return PrintEqual (asm_text, el, vars);
        case CALL:
            return PrintCall  (asm_text, el, vars);
        case RET:
            return PrintRet   (asm_text, el, vars);
        case IF:
        case WHILE:
            return PrintCond  (asm_text, el, vars);
    }

    return 1;
}

bool PrintEqual (FILE* asm_text, element* el, Stack* vars)
{
    print_ass;
    
    if (!(el->left) || !(el->right) || el->left->type != VAR)
        return 1;

    TryPrint (PrintArith, right);

    size_t var_num = VarNumber (vars, el->left->ind);
    if (var_num < 2)
        return 1;
    fprintf (asm_text, "pop [rax + %u]\n", var_num);

    return 0;
}

bool PrintArith (FILE* asm_text, element* el, Stack* vars)
{
    print_ass;

    if (el->type == CALL)
        return PrintCall (asm_text, el, vars);

    if (el->type == NUM)
    {
        fprintf (asm_text, "push %s\n", el->ind);
        return 0;
    }

    if (el->type == VAR)
    {
        size_t var_num = VarNumber (vars, el->ind);
        if (var_num < 2)
            return 1;
        fprintf (asm_text, "push [rax + %d]\n", var_num);
        return 0;
    }

    if (el->type != ARITH)
        return 1;

    TryPrint (PrintArith, left);
    TryPrint (PrintArith, right);

    switch (el->ind[0])
    {
        case '+':
            fprintf (asm_text, "add\n");
            return 0;
        case '-':
            fprintf (asm_text, "sub\n");
            return 0;
        case '*':
            fprintf (asm_text, "mul\n");
            return 0;
        case '/':
            fprintf (asm_text, "div\n");
            return 0;
        case '^':
            fprintf (asm_text, "pow\n");
            return 0;
    }

    return 1;
}

bool PrintCall  (FILE* asm_text, element* el, Stack* vars)
{
    print_ass;

    element* param = el->left;

    while (param)
    {
        if (param->type == NUM)
            fprintf (asm_text, "push %s\n", param->ind);

        else if (param->type == VAR) // ToDo: Delete copypaste.
        {
            size_t var_num = VarNumber (vars, param->ind);
            if (var_num < 2)
                return 1;
            fprintf (asm_text, "push [rax + %d]\n", var_num);
        }

        else
            return 1;

        param = param->left;
    }

    // WARNING: PLACE OF BEATIFIC CODE, NO COPIPASTE!!!
    // Why +3 and +6 - see in GetReturn and GetCond
    
    static const char   sin_str[] = "semen";
    static const char   cos_str[] = "cum";
    static const char print_str[] = "cumming";
    static const char  scan_str[] = "swallow";

    static const size_t   sin_size = sizeof   (sin_str) - 1;
    static const size_t   cos_size = sizeof   (cos_str) - 1;
    static const size_t print_size = sizeof (print_str) - 1;
    static const size_t  scan_size = sizeof  (scan_str) - 1;

    if (sin_size + 6 == el->len && strncmp (sin_str, el->ind + 3, sin_size) == 0)
    {
        fprintf (asm_text, "sin\n");
        return 0;
    }

    if (cos_size + 6 == el->len && strncmp (cos_str, el->ind + 3, cos_size) == 0)
    {
        fprintf (asm_text, "cos\n");
        return 0;
    }

    if (print_size + 6 == el->len && strncmp (print_str, el->ind + 3, print_size) == 0)
    {
        fprintf (asm_text, "out\n");
        return 0;
    }

    if (scan_size + 6 == el->len && strncmp (scan_str, el->ind + 3, scan_size) == 0)
    {
        fprintf (asm_text, "in\n");
        return 0;
    }

    fprintf (asm_text, "call :%s\n", el->ind);
    return 0;
}

bool PrintRet   (FILE* asm_text, element* el, Stack* vars)
{
    print_ass;

    if (el->type != RET)
        return 1;

    TryPrint (PrintArith, left);

    return PrintService (asm_text, "ServiceFiles/FuncEnd.txt");
}

bool PrintCond  (FILE* asm_text, element* el, Stack* vars)
{
    print_ass;

    static size_t cond_number = 0;
    size_t cond_now = cond_number;

    if (el->type != IF && el->type != WHILE)
        return 1;

    if (el->type == WHILE)
        fprintf (asm_text, "while_no%d:\n", cond_now);

    if (PrintComp (asm_text, el->left, vars, cond_now))
        return 1;

    cond_number++;

    if (el->right)
        TryPrint (PrintLR, right);

    cond_number--;

    if (el->type == WHILE)
        fprintf (asm_text, "jmp :while_no%d\n", cond_now);

    fprintf (asm_text, "cond_no%d:\n", cond_now);
    cond_number++;
    return 0;
}

bool PrintComp  (FILE* asm_text, element* el, Stack* vars, size_t cond_number)
{
    print_ass;

    if (el->type != COND)
        return 1;

    TryPrint (PrintArith, left);
    TryPrint (PrintArith, right);

    switch (el->len)
    {
        case 1:
        {
            switch (el->ind[0])
            {
                case '>':
                    fprintf (asm_text, "jbe :cond_no%d\n", cond_number);
                    return 0;
                case '<':
                    fprintf (asm_text, "jae :cond_no%d\n", cond_number);
                    return 0;
                default:
                    return 1;
            }
        }

        case 2:
        {
            switch (el->ind[0])
            {
                case '>':
                    fprintf (asm_text, "jb :cond_no%d\n", cond_number);
                    return 0;
                case '<':
                    fprintf (asm_text, "ja :cond_no%d\n", cond_number);
                    return 0;
                case '!':
                    fprintf (asm_text, "je :cond_no%d\n", cond_number);
                    return 0;
                case '=':
                    fprintf (asm_text, "jne :cond_no%d\n", cond_number);
                    return 0;
                default:
                    return 1;
            }
        }

        default:
            return 1;
    }

    return 1;
}
