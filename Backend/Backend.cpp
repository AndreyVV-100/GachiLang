#include "Backend.h"

int main (int argc, char** argv)
{
    // ToDo: rsp +=, rsp-=
    Tree code = {};

    if (GoTree (&code, (argc > 1) ? argv[1] : "Examples/SqrtTripleDick.gcm"))
    {
        TreeDestructor (&code);
        return 0;
    }
    // CreateGraph (&code);
    GoAsm  (&code);

    TreeDestructor (&code);
    return 0;
}

bool ElfConstructor (Elf* elf)
{
    assert (elf);

    elf->text = (char*) calloc (ALIGN, sizeof (*(elf->text)));
    if (!(elf->text))
        return 1;
    
    elf->printf_.func_name  = PRINT_STR;
    elf-> scanf_.func_name  = SCAN_STR;
    elf->aligned_text_size  = ALIGN;
    return 0;
}

bool ElfDestructor  (Elf* elf)
{
    assert (elf);

    free (elf->text);
    elf->aligned_text_size = 0;
    elf->text_size = 0;
    bool ret = FuncPointerDestructor (elf->functions);
    elf->functions = 0;

    return ret;
}

bool ElfAddBytes    (Elf* elf, const char* bytes, size_t amount_bytes)
{
    assert (elf);
    assert (bytes);

    if (elf->aligned_text_size < elf->text_size + amount_bytes)
        if (ElfResizeText (elf))
            return 1;

    for (size_t i_byte = 0; i_byte < amount_bytes; i_byte++) // ToDo: memcpy
        elf->text[i_byte + elf->text_size] = bytes[i_byte];

    elf->text_size += amount_bytes;
    return 0;
}

bool ElfResizeText  (Elf* elf)
{
    assert (elf);

    elf->text = (char*) realloc (elf->text, elf->aligned_text_size + ALIGN); // sizeof (byte = 1)
    if (!(elf->text)) return 1;
    elf->aligned_text_size += ALIGN;
    return ElfFillZero (elf);
}

bool ElfFillZero    (Elf* elf)
{
    assert (elf);

    if (elf->aligned_text_size < ALIGN)
        return 1;
    
    for (size_t i_byte = 0; i_byte < ALIGN / 4; i_byte++)
        ((size_t*) elf->text)[i_byte] = 0;
    return 0;
}

bool ElfAddPlace    (Elf* elf, const char* func_name, size_t place)
{
    assert (elf);

    FuncPointer* func_p = elf->functions;
    while (func_p)
    {
        if (strcmp (func_p->func_name, func_name) == 0)
            return FuncPointerGetNewPlace (func_p, place);
        
        func_p = func_p->p_next;
    }
    
    if (FuncPointerConstructor (elf, func_name))
        return 1;
    
    return FuncPointerGetNewPlace (elf->functions, place);
}

bool ElfAddPrintf   (Elf* elf, size_t place)
{
    assert (elf);

    return FuncPointerGetNewPlace (&(elf->printf_), place);
}

bool ElfAddScanf    (Elf* elf, size_t place)
{
    assert (elf);

    return FuncPointerGetNewPlace (&(elf->scanf_), place);
}

bool ElfAddFunc     (Elf* elf, const char* func_name)
{
    assert (elf);
    assert (func_name);

    FuncPointer* func_p = elf->functions;
    while (func_p)
    {
        if (strcmp (func_p->func_name, func_name) == 0)
        {
            func_p->func_pl = elf->text_size;
            return 0;
        }

        func_p = func_p->p_next;
    }

    return FuncPointerConstructor (elf, func_name);
}

bool ElfStartFunc   (Elf* elf)
{
    assert (elf);
    return ElfAddBytes (elf, "\x55\x48\x89\xE5\x48\x81\xEC\x80\x00\x00\x00", 11);
    // push rbp
    // mov rbp, rsp
    // sub rsp, 128
}

bool ElfEndFunc     (Elf* elf)
{
    assert (elf);
    return ElfAddBytes (elf, "\x48\x81\xC4\x80\x00\x00\x00\x48\x89\xEC\x5D\xC3", 12);
    // add rsp, 128
    // mov rsp, rbp
    // pop rbp
    // ret
}

bool ElfCreateJumps (Elf* elf)
{
    assert (elf);

    FuncPointer* func_p = elf->functions;

    while (func_p)
    {
        if (func_p->func_pl == __SIZE_MAX__)
        {
            printf ("Error: Function \"%s\" isn't defined.\n", func_p->func_name);
            return 1;
        }

        FuncPointerFillPlaces (func_p, elf->text);
        func_p = func_p->p_next;
    }

    if (elf->printf_.last_pl) // ToDo: Delete copypaste
    {
        char* printf_text  = nullptr;
        size_t printf_size = ReadTxt (&printf_text, "Libraries/printf.gcmlib");
        if (!printf_size)
            return 1;

        elf->printf_.func_pl = elf->text_size;
        if (ElfAddBytes (elf, printf_text, printf_size))
            return 1;

        FuncPointerFillPlaces (&(elf->printf_), elf->text);
        free (printf_text);
    }

    if (elf->scanf_.last_pl) // ToDo: Delete copypaste
    {
        char* scanf_text  = nullptr;
        size_t scanf_size = ReadTxt (&scanf_text, "Libraries/scanf.gcmlib");
        if (!scanf_size)
            return 1;

        elf->scanf_.func_pl = elf->text_size;
        if (ElfAddBytes (elf, scanf_text, scanf_size))
            return 1;

        FuncPointerFillPlaces (&(elf->scanf_), elf->text);
        free (scanf_text);
    }

    return 0;
}

bool ElfWrite       (Elf* elf)
{
    assert (elf);

    system ("cp Backend/ServiceFiles/MagicHeader CompileResult/RightProgram.elf");

    FILE* compiled_file = fopen ("CompileResult/RightProgram.elf", "a+b");
    if (!compiled_file)
    {
        printf ("Error: Unable to create or open file: \"CompileResult/RightProgram.elf\".\n");
        return 1;
    }

    fwrite (&(elf->text_size), sizeof (elf->text_size), 1, compiled_file); // p_filesz
    fwrite (&(elf->text_size), sizeof (elf->text_size), 1, compiled_file); // p_memsz
    fwrite (&ALIGN, sizeof (ALIGN), 1, compiled_file); // p_align
    // fseek  (compiled_file, ALIGN, SEEK_SET); It doesn't work (documentation for a) :-(

    size_t align_need_many_zeroes = (ALIGN - ftell (compiled_file)) / 8;
    for (size_t i_8byte = 0; i_8byte < align_need_many_zeroes; i_8byte++)
        fwrite ("\0\0\0\0\0\0\0\0", 1, 8, compiled_file);

    fwrite (elf->text, sizeof (*(elf->text)), elf->aligned_text_size, compiled_file);
    fclose (compiled_file);
    return 0;
}

bool FuncPointerConstructor (Elf* elf, const char* func_name)
{
    assert (elf);
    assert (func_name);

    FuncPointer* func_p = (FuncPointer*) calloc (1, sizeof (*func_p));
    if (!func_p)
        return 1;
    
    func_p->func_name = func_name;
    func_p->p_next    = elf->functions;
    elf->functions    = func_p;
    func_p->func_pl   = elf->text_size;
    return 0;
}

bool FuncPointerDestructor  (FuncPointer* func_p)
{
    assert (func_p);

    while (func_p)
    {
        PointerPlace* i_place = func_p->last_pl;
        while (i_place)
        {
            PointerPlace* next_place = i_place->next_place;
            *i_place = {};
            free (i_place);
            i_place  = next_place;
        }

        FuncPointer* next_func = func_p->p_next;
        *func_p = {};
        free (func_p);
        func_p = next_func;
    }

    return 0;
}

bool FuncPointerGetNewPlace (FuncPointer* func_p, size_t place)
{
    assert (func_p);

    PointerPlace* new_place = (PointerPlace*) calloc (1, sizeof (*new_place));
    if (!new_place)
        return 1;
    
    new_place->point = place;
    new_place->next_place = func_p->last_pl;
    func_p->last_pl = new_place;
    return 0;
}

void FuncPointerFillPlaces  (FuncPointer* func_p, char* text)
{
    assert (func_p);
    assert (text);

    int func_addr = (int) func_p->func_pl;
    PointerPlace* i_place = func_p->last_pl;

    while (i_place)
    {
        *(int*) (text + i_place->point) = func_addr - 4 - i_place->point; // 4 - size of place
        i_place = i_place->next_place;
    }
    return;
}

void GoAsm (Tree* code)
{
    assert (code);

    Elf elf = {};
    if (ElfConstructor (&elf))
        return;
    
    // call sex
    // mov rax, 0x3C
    // xor rdi, rdi
    // syscall
    ElfAddBytes (&elf, "\xE8\x00\x00\x00\x00\x48\xC7\xC0\x3C\x00\x00\x00\x48\x31\xFF\x0F\x05", 17);
    ElfAddPlace (&elf, "♂sex♂", 1);

    if (PrintDec (&elf, code->head))
        printf ("Something went wrong...\n");

    ElfCreateJumps (&elf);
    ElfWrite (&elf);
    system ("chmod u+x CompileResult/RightProgram.elf");
    ElfDestructor (&elf);
    return;
}

/* bool PrintService (Elf* elf, const char* file_path) // ToDo: Not actually
{
    assert (asm_text);

    char* asm_begin = nullptr;
    
    if (!ReadTxt (&asm_begin, file_path))
        return 1;

    fprintf (asm_text, "%s", asm_begin);
    free (asm_begin);
    return 0; // ToDo: There was no return!
} */

/* void DeleteFuncSpaces (element* el) // ToDo: Not actually
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
} */

bool PrintDec (Elf* elf, element* el)
{
    assert (elf);
    assert (el);
    
    if (el->type != DEC)
        return 1;

    if (PrintFunc (elf, el->left))
        return 1;

    if (el->right && PrintDec (elf, el->right))
        return 1;

    return 0;
}

bool PrintFunc (Elf* elf, element* el)
{
    assert (elf);
    assert (el);

    if (el->type != FUNC)
        return 1;

    ElfAddFunc (elf, el->ind);
    XMM_COUNTER = 0;

    const size_t max_var_num = 32;
    Stack vars_ = {};
    StackConstructor (&vars_, max_var_num);

    char var_count = 0xF8;
    if (TakeFuncVars (el, &vars_, &var_count)) // ToDo: See, maybe OK
    {
        StackDestructor (&vars_);
        return 1;
    }

    Stack* vars = &vars_;
    ElfStartFunc (elf);

    // TryPrint (PrintParam, left); // ToDo: Only remember
    TryPrint (PrintLR,   right);

    ElfEndFunc (elf);
    return 0;
}

bool TakeFuncVars (element* el, Stack* vars, char* var_count)
{
    assert (vars);
    if (!el)
        return 0;

    element* el_now  = el;
    char param_count = 0x10; // ToDo: Magic const

    while (el_now->type == PARAM)
    {
        if (el->left && el->left->type != PARAM)
        {
            printf ("TakeFuncVars error: bad parameters.\n");
            return 1;
        }

        el_now->var_pos = param_count;
        StackPush (vars, *el_now);

        if (param_count == 0x78)
            printf ("Warning: Maybe parameter counter overfull.\n");
        param_count += 8;

        if (el_now->left == nullptr)
            return 0;
        else
            el_now = el_now->left;
    }

    if (el->type == EQUAL)
    {
        if (!(el->left) || el->left->type != VAR)
        {
            printf ("TakeFuncVars error: bad equality.\n");
            return 1;
        }

        if (VarNumber (vars, el->left->ind) != 1) // ToDo: Magic const
            return 0;

        el->left->var_pos = *var_count;
        StackPush (vars, *(el->left));

        if (*var_count == 0x80)
            printf ("Warning: Maybe variable counter overfull.\n");
        *var_count -= 8;

        return 0;
    }

    if (TakeFuncVars (el->left, vars, var_count))
        return 1;
    return TakeFuncVars (el->right, vars, var_count);
}

/* bool PrintParam (Elf* elf, element* el, Stack* vars) // ToDo: Not actually
{
    assert (elf);
    assert (vars);

    if (PrintService (asm_text, "Backend/ServiceFiles/FuncBegin.txt")) // ToDo: Create stack frame
        return 1;

    fprintf (asm_text, "\n""push %lu\n"
                           "pop [rax + 1]\n",
             vars->size + 2);

    if (SaveParam (elf, el, vars))
        return 1;

    return 0;
}
 */

/* bool SaveParam (Elf* elf, element* el, Stack* vars) // ToDo: Not actually
{
    assert (elf);
    assert (vars);
    if (!el)
        return 0;

    TryPrint (SaveParam, left);
    size_t var_num = VarNumber (vars, el->ind); // ToDo: How it works?
    if (var_num < 2)
        return 1;
    fprintf (asm_text, "pop [rax + %lu]\n", var_num);
    return 0;
} */

char VarNumber (Stack* vars, const char* var_name)
{
    assert (vars);
    assert (var_name);

    for (size_t result = 0; result < vars->size; result++)
        if (strcmp (var_name, vars->buffer[result].ind) == 0)
            return vars->buffer[result].var_pos;

    // printf ("Error: No var with name \"%s\".\n", var_name);
    return 1; // ToDo: Error const
}

bool PrintLR    (Elf* elf, element* el, Stack* vars)
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

    XMM_COUNTER = 0;

    switch (el->type)
    {
        case EQUAL:
            return PrintEqual (elf, el, vars);
        case CALL:
            return PrintCall  (elf, el, vars);
        case RET:
            return PrintRet   (elf, el, vars);
        case IF:
        case WHILE:
            return PrintCond  (elf, el, vars);
        default:
            return 1;
    }

    return 1;
}

bool PrintEqual (Elf* elf, element* el, Stack* vars)
{
    print_ass;
    
    if (!(el->left) || !(el->right) || el->left->type != VAR)
        return 1;

    TryPrint (PrintArith, right); // ToDo: xmm counter

    char var_num = VarNumber (vars, el->left->ind);

    ElfAddBytes (elf, "\xF2\x0F\x11\x45", 4); // ToDo: In xmm0?
    return ElfAddBytes (elf, &var_num, 1);
}

bool PrintArith (Elf* elf, element* el, Stack* vars) // ToDo: Functions
{
    print_ass;

    if (el->type == CALL)
    {
        if (PrintCall (elf, el, vars))
            return 1;

/*  This is in PrintCall
        // movq xmm?, rax
        ElfAddBytes (elf, (XMM_COUNTER < 8) ? "\x66\x48\x0F\x6E" : "\x66\x4C\x0F\x6E", 4);

        char num_register = (XMM_COUNTER % 8) * 8 + 0xC0;
        XMM_COUNTER++;
        ElfAddBytes (elf, &num_register, 1); */
        return 0;
    }

    if (el->type == NUM)
    {
        double num = atof (el->ind);
        // mov rax, number
        ElfAddBytes (elf, "\x48\xB8", 2);
        ElfAddBytes (elf, (char*) &num, 8);

        // movq xmm?, rax
        ElfAddBytes (elf, (XMM_COUNTER < 8) ? "\x66\x48\x0F\x6E" : "\x66\x4C\x0F\x6E", 4);

        char num_register = (XMM_COUNTER % 8) * 8 + 0xC0;
        XMM_COUNTER++;
        ElfAddBytes (elf, &num_register, 1);
        return 0;
    }

    if (el->type == VAR)
    {
        char var_num = VarNumber (vars, el->ind);

        if (XMM_COUNTER < 8)
            ElfAddBytes (elf, "\xF2\x0F\x10", 3);
        else
            ElfAddBytes (elf, "\xF2\x44\x0F\x10", 4);

        char num_register = (XMM_COUNTER % 8) * 8 + 0x45;
        XMM_COUNTER++;

        ElfAddBytes (elf, &num_register, 1);
        ElfAddBytes (elf, &var_num, 1);
        // fprintf (asm_text, "push [rax + %lu]\n", var_num); // ToDo: movsd xmm0, qword [rbp + %lu]
        return 0;
    }

    if (el->type != ARITH)
        return 1;

    TryPrint (PrintArith, left);
    TryPrint (PrintArith, right);

    // command xmm(№-1), xmm№
    // command = {addsd, subsd, mulsd, divsd} 

    if (XMM_COUNTER < 8)
        ElfAddBytes (elf, "\xF2\x0F", 2);
    else if (XMM_COUNTER == 8)
        ElfAddBytes (elf, "\xF2\x41\x0F", 3);
    else
        ElfAddBytes (elf, "\xF2\x45\x0F", 3);

    switch (el->ind[0])
    {
        case '+':
            ElfAddBytes (elf, "\x58", 1);
            break;
        case '-':
            ElfAddBytes (elf, "\x5C", 1);
            break;
        case '*':
            ElfAddBytes (elf, "\x59", 1);
            break;
        case '/':
            ElfAddBytes (elf, "\x5E", 1);
            break;
        // case '^': // ToDo: Delete operator
            // fprintf (asm_text, "pow\n");
            // return 0;
        default:
            return 1;
    }

    char num_register = ((XMM_COUNTER - 2) % 8) * 8 + (XMM_COUNTER - 1) % 8 + 0xC0;
    XMM_COUNTER--;
    ElfAddBytes (elf, &num_register, 1);
    return 0;
}

bool PrintCall  (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    int push_shift = 0; // number of pushes on bytes

    if (PrintCallParam (elf, el->left, vars, &push_shift))
        return 1;

    // Old code:

    /*
    element* param = el->left;

    while (param)
    {
        if (param->type == NUM)
            fprintf (asm_text, "push %s\n", param->ind); 

        else if (param->type == VAR)
        {
            size_t var_num = VarNumber (vars, param->ind);
            if (var_num < 2)
                return 1;
            fprintf (asm_text, "push [rax + %lu]\n", var_num);
        }

        else
            return 1;

        param = param->left;
    } */

/* 
    const size_t without_backslash_0 = 1;
    
    static const size_t   sin_size = sizeof   (SIN_STR) - without_backslash_0;
    static const size_t   cos_size = sizeof   (COS_STR) - without_backslash_0;
    static const size_t print_size = sizeof (PRINT_STR) - without_backslash_0;
    static const size_t  scan_size = sizeof  (SCAN_STR) - without_backslash_0;

    if (sin_size + 2 * MALE_LEN == el->len && strncmp (SIN_STR, el->ind + MALE_LEN, sin_size) == 0)
    {
        fprintf (asm_text, "sin\n");
        return 0;
    }

    if (cos_size + 2 * MALE_LEN == el->len && strncmp (COS_STR, el->ind + MALE_LEN, cos_size) == 0)
    {
        fprintf (asm_text, "cos\n");
        return 0;
    }

    if (print_size + 2 * MALE_LEN == el->len && strncmp (PRINT_STR, el->ind + MALE_LEN, print_size) == 0)
    {
        fprintf (asm_text, "out\n");
        return 0;
    }

    if (scan_size + 2 * MALE_LEN == el->len && strncmp (SCAN_STR, el->ind + MALE_LEN, scan_size) == 0)
    {
        fprintf (asm_text, "in\n");
        return 0;
    } */
/* ToDo: Warning: Deleted functions, but needed
    if (strcmp (SIN_STR, el->ind) == 0)
    {
        fprintf (asm_text, "sin\n");
        return 0;
    }

    if (strcmp (COS_STR, el->ind) == 0)
    {
        fprintf (asm_text, "cos\n");
        return 0;
    }
 */

    // ToDo: Strcmp to ElfAddPlace

    if (strcmp (PRINT_STR, el->ind) == 0)
    {
        // fprintf (asm_text, "out\n");
        ElfAddPrintf (elf, elf->text_size + 1);
        // return 0;
    }

    else if (strcmp (SCAN_STR, el->ind) == 0)
    {
        // fprintf (asm_text, "in\n");
        ElfAddScanf (elf, elf->text_size + 1);
        // return 0;
    }
    
    else
        ElfAddPlace (elf, el->ind, elf->text_size + 1);

    // call func
    ElfAddBytes (elf, "\xE8\x00\x00\x00\x00", 5);
    
    // This is delete pushes
    // add rsp, push_shift
    ElfAddBytes (elf, "\x48\x81\xC4", 3);
    ElfAddBytes (elf, (char*) &push_shift, 4);

    // ToDo: Delete copypaste
    // movq xmm?, rax
    ElfAddBytes (elf, (XMM_COUNTER < 8) ? "\x66\x48\x0F\x6E" : "\x66\x4C\x0F\x6E", 4);

    char num_register = (XMM_COUNTER % 8) * 8 + 0xC0;
    XMM_COUNTER++;
    ElfAddBytes (elf, &num_register, 1);

    // fprintf (asm_text, "call :%s\n", el->ind); // ToDo: label work    
    return 0;
}

bool PrintRet   (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->type != RET)
        return 1;

    TryPrint (PrintArith, left);

    // movq rax, xmm0
    ElfAddBytes (elf, "\x66\x48\x0F\x7E\xC0", 5);
    // return PrintService (asm_text, "Backend/ServiceFiles/FuncEnd.txt"); // ToDo: end of func
    return ElfEndFunc (elf);
}

bool PrintCond  (Elf* elf, element* el, Stack* vars)
{
    print_ass;
/* 
    static size_t cond_number = 0;
    size_t cond_now = cond_number; // ToDo: Save position
    cond_number++;
 */
    if (el->type != IF && el->type != WHILE)
        return 1;
    
    int start_of_while = (int) (elf->text_size);

    /* if (el->type == WHILE)
        fprintf (asm_text, "while_no%lu:\n", cond_now); */

    if (PrintComp (elf, el->left, vars)) 
            return 1;

    size_t jump_from_place = elf->text_size; // ToDo: Magic const

    if (el->right)
        TryPrint (PrintLR, right);

    if (el->type == WHILE)
    {   
        // jmp start_of_while
        ElfAddBytes (elf, "\xE9", 1);
        start_of_while -= elf->text_size + 4;
        ElfAddBytes (elf, (char*) &start_of_while, 4);
    }

    /* if (el->type == WHILE)
        fprintf (asm_text, "jmp :while_no%lu\n", cond_now); // ToDo: if -> jump */

    // fprintf (asm_text, "cond_no%lu:\n", cond_now); // ToDo: relative jump

    *((int*) (elf->text + jump_from_place - 4)) = (int) (elf->text_size - jump_from_place);
    return 0;
}

bool PrintComp  (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->type != COND)
        return 1;

    TryPrint (PrintArith, left);
    // movq rdx, xmm0
    ElfAddBytes (elf, "\x66\x48\x0F\x7E\xC2", 5);

    XMM_COUNTER = 0;
    TryPrint (PrintArith, right);
    // movq xmm1, rdx
    ElfAddBytes (elf, "\x66\x48\x0F\x6E\xCA", 5);

    switch (el->len)
    {
        case 1:
        {
            switch (el->ind[0])
            {
                case '>':
                    // vcmpsd xmm2, xmm1, xmm0, 2
                    // movq rax, xmm2
                    // cmp rax, 0
                    // jne ?

                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x02\x66\x48\x0F\x7E\xD0\x48\x83\xF8\x00\x0F\x85\x00\x00\x00\x00", 20);
                    // fprintf (asm_text, "jbe :cond_no%lu\n", cond_number);
                    return 0;

                case '<':
                    // vcmpsd xmm2, xmm1, xmm0, 0x0D
                    // movq rax, xmm2
                    // cmp rax, 0
                    // jne ?
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x0D\x66\x48\x0F\x7E\xD0\x48\x83\xF8\x00\x0F\x85\x00\x00\x00\x00", 20);
                    // fprintf (asm_text, "jae :cond_no%lu\n", cond_number);
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
                    // vcmpsd xmm2, xmm1, xmm0, 1
                    // movq rax, xmm2
                    // cmp rax, 0
                    // jne ?
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x01\x66\x48\x0F\x7E\xD0\x48\x83\xF8\x00\x0F\x85\x00\x00\x00\x00", 20);
                    // fprintf (asm_text, "jb :cond_no%lu\n", cond_number);
                    return 0;

                case '<':
                    // vcmpsd xmm2, xmm1, xmm0, 0x0E
                    // movq rax, xmm2
                    // cmp rax, 0
                    // jne ?
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x0E\x66\x48\x0F\x7E\xD0\x48\x83\xF8\x00\x0F\x85\x00\x00\x00\x00", 20);
                    // fprintf (asm_text, "ja :cond_no%lu\n", cond_number);
                    return 0;

                case '!':
                    // vcmpsd xmm2, xmm1, xmm0, 0
                    // movq rax, xmm2
                    // cmp rax, 0
                    // jne ?
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x00\x66\x48\x0F\x7E\xD0\x48\x83\xF8\x00\x0F\x85\x00\x00\x00\x00", 20);
                    // fprintf (asm_text, "je :cond_no%lu\n", cond_number);
                    return 0;

                case '=':
                    // vcmpsd xmm2, xmm1, xmm0, 4
                    // movq rax, xmm2
                    // cmp rax, 0
                    // jne ?
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x04\x66\x48\x0F\x7E\xD0\x48\x83\xF8\x00\x0F\x85\x00\x00\x00\x00", 20);
                    // fprintf (asm_text, "jne :cond_no%lu\n", cond_number);
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

bool PrintCallParam (Elf* elf, element* el, Stack* vars, int* push_shift)
{
    if (!el)
        return 0;

    if (PrintCallParam (elf, el->left, vars, push_shift))
        return 1;

    if (el->type == NUM)
    {
        double num = atof (el->ind);
        // mov rax, num
        ElfAddBytes (elf, "\x48\xB8", 2);
        ElfAddBytes (elf, (char*) &num, 8);
    }
        // fprintf (asm_text, "push %s\n", param->ind); // ToDo: mov [rsp + x], num

    else if (el->type == VAR) // ToDo: Delete copypaste.
    {
        char var_num = VarNumber (vars, el->ind);
        // mov rax, qword [rbp + var_num]
        ElfAddBytes (elf, "\x48\x8B\x45", 3);
        ElfAddBytes (elf, &var_num, 1);
        // fprintf (asm_text, "push [rax + %lu]\n", var_num); // ToDo: mov [rsp + x], [rbp + %lu]
    }

    else
        return 1;

    // push rax
    ElfAddBytes (elf, "\x50", 1);
    *push_shift += 8;
    return 0;
}
