#include "../Tree/Stack.h"

#define print_ass assert (elf);   \
                  assert (el);    \
                  assert (vars)

#define TryPrint(func, way) if (func (elf, el->way, vars))      \
                            {                                   \
                                printf ("Print error in function %s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
                                printf ("It was on point %p in element: %s\n", el, el->ind);             \
                                return 1;                       \
                            }

const size_t ALIGN = 0x1000;
char   XMM_COUNTER = 0;

struct PointerPlace;

struct FuncPointer
{
    const char* func_name = nullptr;
    size_t        func_pl = __SIZE_MAX__;
    PointerPlace* last_pl = nullptr; //! Last needed place
    FuncPointer*   p_next = nullptr;
};

struct Elf
{
    FuncPointer printf_ = {};
    FuncPointer  scanf_ = {};

    char* text = nullptr;
    FuncPointer*   functions = nullptr;
    size_t         text_size = 0;
    size_t aligned_text_size = 0;
};

struct PointerPlace
{
    size_t point = __SIZE_MAX__;
    PointerPlace* next_place = nullptr;
};

// ToDo: Write functions

bool ElfConstructor (Elf* elf);

bool ElfDestructor  (Elf* elf);

bool ElfAddBytes    (Elf* elf, const char* bytes, size_t amount_bytes);

bool ElfResizeText  (Elf* elf);

bool ElfFillZero    (Elf* elf);

bool ElfAddPlace    (Elf* elf, const char* func_name, size_t place);

bool ElfAddPrintf   (Elf* elf, size_t place);

bool ElfAddScanf    (Elf* elf, size_t place);

bool ElfAddFunc     (Elf* elf, const char* func_name);

bool ElfStartFunc   (Elf* elf);

bool ElfEndFunc     (Elf* elf);

bool ElfCreateJumps (Elf* elf);

bool ElfWrite       (Elf* elf);

bool FuncPointerConstructor (Elf* elf, const char* func_name);

bool FuncPointerDestructor  (FuncPointer* func_p);

bool FuncPointerGetNewPlace (FuncPointer* func_p, size_t place);

void FuncPointerFillPlaces  (FuncPointer* func_p, char* text);

// ?

bool ElfDump (Elf* elf);

bool ElfLog  (Elf* elf);

// ToDo: Rewrite functions

void GoAsm (Tree* tree);

bool PrintService (Elf* elf, const char* file_path);

// void DeleteFuncSpaces (element* el);

bool PrintDec  (Elf* elf, element* el);

bool PrintFunc (Elf* elf, element * el);

bool TakeFuncVars (element * el, Stack * vars, char* var_count);

bool PrintParam (Elf* elf, element * el, Stack * vars);

bool SaveParam  (Elf* elf, element * el, Stack * vars);

char VarNumber (Stack * vars, const char* var_name);

bool PrintLR    (Elf* elf, element * el, Stack * vars);

bool PrintEqual (Elf* elf, element * el, Stack * vars);

bool PrintArith (Elf* elf, element * el, Stack * vars);

bool PrintCall  (Elf* elf, element * el, Stack * vars);

bool PrintRet   (Elf* elf, element * el, Stack * vars);

bool PrintCond  (Elf* elf, element * el, Stack * vars);

bool PrintComp  (Elf* elf, element * el, Stack * vars);

bool PrintCallParam (Elf* elf, element* el, Stack* vars, int* push_shift);
