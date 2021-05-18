#include "Backend.h"

int main()
{
    Tree code = {};
    TreeConstructor (&code);

    if (LoadTree (&code))
        GoElf (&code);

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

    memcpy (elf->text + elf->text_size, bytes, amount_bytes);
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

    if (strcmp (PRINT_STR, func_name) == 0)
        return ElfAddPrintf (elf, place);

    if (strcmp (SCAN_STR, func_name)  == 0)
        return ElfAddScanf  (elf, place);

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
    return ElfAddBytes (elf, "\x55"                          // push rbp    
                             "\x48\x89\xE5"                  // mov rbp, rsp
                             "\x48\x81\xEC\x80\x00\x00\x00", // sub rsp, 128
                             11);
    // ToDo: rsp +=, rsp-=
}

bool ElfEndFunc     (Elf* elf)
{
    assert (elf);
    return ElfAddBytes (elf, "\x48\x89\xEC" // mov rsp, rbp
                             "\x5D"         // pop rbp
                             "\xC3",        // ret
                             5);
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

    if (elf->printf_.last_pl)
        if (ElfAddLibrary (elf, "Libraries/printf.gcmlib", &(elf->printf_)))
            return 1;

    if (elf->scanf_.last_pl)
        if (ElfAddLibrary (elf, "Libraries/scanf.gcmlib", &(elf->scanf_)))
            return 1;

    return 0;
}

bool ElfAddLibrary  (Elf* elf, const char* lib_name, FuncPointer* lib_p)
{
    char* lib_text  = nullptr;
    size_t lib_size = ReadTxt (&lib_text, lib_name);
    if (!lib_size)
        return 1;

    lib_p->func_pl = elf->text_size;
    if (ElfAddBytes (elf, lib_text, lib_size))
        return 1;

    FuncPointerFillPlaces (lib_p, elf->text);
    free (lib_text);
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
        *(int*) (text + i_place->point) = func_addr - JMP_NUM_SIZE - i_place->point;
        i_place = i_place->next_place;
    }
    return;
}

void GoElf (Tree* code)
{
    assert (code);

    Elf elf = {};
    if (ElfConstructor (&elf))
        return;
    
    ElfAddBytes (&elf, "\xE8\x00\x00\x00\x00"           // call ♂sex♂
                       "\x48\xC7\xC0\x3C\x00\x00\x00"   // mov rax, 0x3C
                       "\x48\x31\xFF"                   // xor rdi, rdi
                       "\x0F\x05",                      // syscall
                       17);

    ElfAddPlace (&elf, "♂sex♂", 1);

    if (PrintDec (&elf, code->head))
        printf ("Something went wrong...\n");

    ElfCreateJumps (&elf);
    ElfWrite (&elf);
    system ("chmod u+x CompileResult/RightProgram.elf");
    ElfDestructor (&elf);
    return;
}

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
    elf->xmm_counter = 0;

    const size_t max_var_num = 32;
    Stack vars_ = {};
    StackConstructor (&vars_, max_var_num);

    char var_count = FIRST_LOCAL_VAR;
    if (TakeFuncVars (el, &vars_, &var_count))
    {
        StackDestructor (&vars_);
        return 1;
    }

    Stack* vars = &vars_;
    ElfStartFunc (elf);
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
    char param_count = FIRST_PARAMETER; 
    // First param - [rbp + 0x10], second - [rbp + 0x18] and ect.

    while (el_now->type == PARAM)
    {
        if (el->left && el->left->type != PARAM)
        {
            printf ("TakeFuncVars error: bad parameters.\n");
            return 1;
        }

        el_now->var_pos = param_count;
        StackPush (vars, *el_now);

        if (param_count == MAX_PARAMETER)
            printf ("Warning: Maybe parameter counter overfull.\n");
        param_count += SIZEOF_ALIGN;

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

        if (VarNumber (vars, el->left->ind) != VAR_WASNT_CREATED)
            return 0;

        el->left->var_pos = *var_count;
        StackPush (vars, *(el->left));

        if (*var_count == MAX_LOCAL_VAR)
            printf ("Warning: Maybe variable counter overfull.\n");
        *var_count -= SIZEOF_ALIGN;

        return 0;
    }

    if (TakeFuncVars (el->left, vars, var_count))
        return 1;
    return TakeFuncVars (el->right, vars, var_count);
}

char VarNumber (Stack* vars, const char* var_name)
{
    assert (vars);
    assert (var_name);

    for (size_t result = 0; result < vars->size; result++)
        if (strcmp (var_name, vars->buffer[result].ind) == 0)
            return vars->buffer[result].var_pos;

    return VAR_WASNT_CREATED;
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

    elf->xmm_counter = 0;

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

    TryPrint (PrintArith, right);

    char var_num = VarNumber (vars, el->left->ind);
    if (var_num == VAR_WASNT_CREATED)
        return 1;

    ElfAddBytes (elf, "\xF2\x0F\x11\x45", 4); // movsd [rbp - 8*№], xmm0
    return ElfAddBytes (elf, &var_num, 1);
}

bool PrintArith (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->type == CALL)
        return PrintCall (elf, el, vars);

    if (el->type == NUM)
        return PrintArithNum (elf, el);

    if (el->type == VAR)
        return PrintArithVar (elf, el, vars);

    return PrintArithOper (elf, el, vars);
}

bool PrintCall  (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (strcmp (SQRT_STR, el->ind) == 0)
        return PrintSqrt (elf, el, vars);

    PrintSaveXMM (elf);

    int push_shift = 0; // number of pushes on bytes
    if (PrintCallParam (elf, el->left, vars, &push_shift))
        return 1;

    // ToDo: Sin, cos, pow, sqrt
    
    ElfAddPlace (elf, el->ind, elf->text_size + 1);
    ElfAddBytes (elf, "\xE8\x00\x00\x00\x00", 5); // call 00 00 00 00
    
    // This is delete pushes
    ElfAddBytes (elf, "\x48\x81\xC4", 3); // add rsp, push_shift
    ElfAddBytes (elf, (char*) &push_shift, 4);
    mov_xmm_rax // movq xmm?, rax

    PrintLoadXMM (elf);
    return 0;
}

bool PrintRet   (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->type != RET)
        return 1;

    TryPrint (PrintArith, left);

    ElfAddBytes (elf, "\x66\x48\x0F\x7E\xC0", 5); // movq rax, xmm0
    return ElfEndFunc (elf);
}

bool PrintCond  (Elf* elf, element* el, Stack* vars)
{
    print_ass;
    if (el->type != IF && el->type != WHILE)
        return 1;
    
    int start_of_while = (int) (elf->text_size);

    if (PrintComp (elf, el->left, vars)) 
            return 1;

    size_t jump_from_place = elf->text_size;

    if (el->right)
        TryPrint (PrintLR, right);

    if (el->type == WHILE)
    {   
        ElfAddBytes (elf, "\xE9", 1); // 0xE9 = jmp
        start_of_while -= elf->text_size + JMP_NUM_SIZE;
        ElfAddBytes (elf, (char*) &start_of_while, JMP_NUM_SIZE);
    }

    // Place of conditional jump
    *((int*) (elf->text + jump_from_place - JMP_NUM_SIZE)) = (int) (elf->text_size - jump_from_place);
    return 0;
}

bool PrintComp  (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->type != COND)
        return 1;

    TryPrint (PrintArith, left);
    ElfAddBytes (elf, "\x66\x48\x0F\x7E\xC2", 5); // movq rdx, xmm0

    elf->xmm_counter = 0;
    TryPrint (PrintArith, right);
    ElfAddBytes (elf, "\x66\x48\x0F\x6E\xCA", 5); // movq xmm1, rdx

    switch (el->len)
    {
        case 1:
        {
            switch (el->ind[0])
            {
                case '>':
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x02", 5); // vcmpsd xmm2, xmm1, xmm0, 2
                    break;

                case '<':
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x0D", 5); // vcmpsd xmm2, xmm1, xmm0, 0x0D
                    break;

                default:
                    return 1;
            }
            break;
        }

        case 2:
        {
            switch (el->ind[0])
            {
                case '>':
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x01", 5); // vcmpsd xmm2, xmm1, xmm0, 1
                    break;

                case '<':
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x0E", 5); // vcmpsd xmm2, xmm1, xmm0, 0x0E
                    break;

                case '!':
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x00", 5); // vcmpsd xmm2, xmm1, xmm0, 0
                    break;

                case '=':
                    ElfAddBytes (elf, "\xC5\xF3\xC2\xD0\x04", 5); // vcmpsd xmm2, xmm1, xmm0, 4
                    break;

                default:
                    return 1;
            }
            break;
        }

        default:
            return 1;
    }

    ElfAddBytes (elf, "\x66\x48\x0F\x7E\xD0"        // movq rax, xmm2
                      "\x48\x83\xF8\x00"            // cmp rax, 0
                      "\x0F\x85\x00\x00\x00\x00",   // jne 00 00 00 00 (will be patched)
                      15);

    return 0;
}

bool PrintCallParam (Elf* elf, element* el, Stack* vars, int* push_shift)
{
    assert (elf);
    assert (vars);
    assert (push_shift);
    if (!el)
        return 0;

    if (PrintCallParam (elf, el->left, vars, push_shift))
        return 1;

    if (el->type == NUM)
    {
        double num = atof (el->ind);
        ElfAddBytes (elf, "\x48\xB8", 2); // mov rax, num
        ElfAddBytes (elf, (char*) &num, 8);
    }

    else if (el->type == VAR)
    {
        char var_num = VarNumber (vars, el->ind);
        if (var_num == VAR_WASNT_CREATED)
            return 1;

        ElfAddBytes (elf, "\x48\x8B\x45", 3); // mov rax, qword [rbp + var_num]
        ElfAddBytes (elf, &var_num, 1);
    }

    else
        return 1;

    ElfAddBytes (elf, "\x50", 1); // \x50 = push rax
    *push_shift += 8;
    return 0;
}

bool PrintArithNum  (Elf* elf, element* el)
{
    assert (elf);
    assert (el);

    double num = atof (el->ind);
    ElfAddBytes (elf, "\x48\xB8", 2); // mov rax, number
    ElfAddBytes (elf, (char*) &num, 8);
    mov_xmm_rax // movq xmm?, rax

    return 0;
}

bool PrintArithVar  (Elf* elf, element* el, Stack* vars)
{
    print_ass;
    if (elf->xmm_counter < XMM_MAX)
    {
        char var_num = VarNumber (vars, el->ind);
        if (var_num == VAR_WASNT_CREATED)
            return 1;

        if (elf->xmm_counter < 8)
            ElfAddBytes (elf, "\xF2\x0F\x10", 3); // movsd xmm?, qword [rbp - 8*№]
        else
            ElfAddBytes (elf, "\xF2\x44\x0F\x10", 4); // movsd xmm?, qword [rbp - 8*№]

        char num_register = (elf->xmm_counter % 8) * 8 + 0x45;
        elf->xmm_counter += 1;

        ElfAddBytes (elf, &num_register, 1);
        ElfAddBytes (elf, &var_num, 1);
    }
    else
    {
        printf ("Fucking slave, you are bad programmer! Your math expression is so big.\n");
        return 1;
    }
    
    return 0;
}

bool PrintArithOper (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->type != ARITH)
        return 1;

    TryPrint (PrintArith, left);
    TryPrint (PrintArith, right);

    // arithm_op xmm(№-1), xmm№
    // arithm_op = {addsd, subsd, mulsd, divsd} 

    if (elf->xmm_counter < 8)
        ElfAddBytes (elf, "\xF2\x0F", 2); // arithm_op xmm[0..6], xmm[1..7]
    else if (elf->xmm_counter == 8)
        ElfAddBytes (elf, "\xF2\x41\x0F", 3); // arithm_op xmm7, xmm8
    else
        ElfAddBytes (elf, "\xF2\x45\x0F", 3); // arithm_op xmm[8..14], xmm[9..15]

    switch (el->ind[0])
    {
        case '+':
            ElfAddBytes (elf, "\x58", 1); // arithm_op = addsd
            break;
        case '-':
            ElfAddBytes (elf, "\x5C", 1); // arithm_op = subsd
            break;
        case '*':
            ElfAddBytes (elf, "\x59", 1); // arithm_op = mulsd
            break;
        case '/':
            ElfAddBytes (elf, "\x5E", 1); // arithm_op = divsd
            break;
        // case '^': // ToDo: Deleted operator
            // fprintf (asm_text, "pow\n");
            // return 0;
        default:
            return 1;
    }

    char num_registers = ((elf->xmm_counter - 2) % 8) * 8 + (elf->xmm_counter - 1) % 8 + 0xC0;
    elf->xmm_counter -= 1;
    ElfAddBytes (elf, &num_registers, 1);
    return 0;
}

bool PrintSaveXMM   (Elf* elf)
{
    assert (elf);

    for (char i_xmm = elf->xmm_counter - 1; i_xmm >= 0; i_xmm--)
    {
        ElfAddBytes (elf, (i_xmm < 8) ? "\x66\x48\x0F\x7E" : "\x66\x4C\x0F\x7E", 4); // movq rax, xmm?
        char num_register = (i_xmm % 8) * 8 + 0xC0;
        ElfAddBytes (elf, &num_register, 1); 
        ElfAddBytes (elf, "\x50", 1); // \x50 = push rax
    }

    return 0;
}

bool PrintLoadXMM   (Elf* elf)
{
    assert (elf);

    for (char i_xmm = 0; i_xmm < elf->xmm_counter - 1; i_xmm++) // - 1 --- arg was added
    {
        ElfAddBytes (elf, "\x58", 1); // 0x58 = pop rax

        ElfAddBytes (elf, (i_xmm < 8) ? "\x66\x48\x0F\x6E": // movq xmm?, rax
                                        "\x66\x4C\x0F\x6E", // movq xmm?, rax
                                        4);
        char num_register = (i_xmm % 8) * 8 + 0xC0;
        ElfAddBytes (elf, &num_register, 1);
    }

    return 0;
}

bool PrintSqrt (Elf* elf, element* el, Stack* vars)
{
    print_ass;

    if (el->left->type == NUM)
    {
        if (PrintArithNum (elf, el->left))
            return 1;
    }

    else if (el->left->type == VAR)
    {
        if (PrintArithVar (elf, el->left, vars))
            return 1;
    }
    
    else return 1;

    // sqrtsd xmm(№-1), xmm№
    char num_register = ((elf->xmm_counter - 1) % 8) * 8 + 0xC0;
    if (elf->xmm_counter <= 8)
        ElfAddBytes (elf, "\xF2\x0F\x51", 3); // sqrtsd xmm[0:6], xmm[1:7]
    if (elf->xmm_counter == 8)
        ElfAddBytes (elf, "\xF2\x41\x0F\x51", 4); // sqrtsd xmm7, xmm8
    else
        ElfAddBytes (elf, "\xF2\x45\x0F\x51", 4); // sqrtsd xmm[8:14], xmm[9:15]
    
    return ElfAddBytes (elf, &num_register, 1);
}
