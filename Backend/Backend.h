#include "../TreeBase/Tree.h"

#define print_ass assert (elf);   \
                  assert (el);    \
                  assert (vars)

#define TryPrint(func, way) if (func (elf, el->way, vars))      \
                            {                                   \
                                printf ("Print error in function %s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
                                printf ("It was on point %p in element: %s\n", el, el->ind);                     \
                                return 1;                       \
                            }

#define mov_xmm_rax if (elf->xmm_counter < XMM_MAX)                                                                 \
                    {                                                                                               \
                        ElfAddBytes (elf, (elf->xmm_counter < 8) ? "\x66\x48\x0F\x6E" : "\x66\x4C\x0F\x6E", 4);     \
                        char num_register = (elf->xmm_counter % 8) * 8 + 0xC0;                                      \
                        elf->xmm_counter += 1;                                                                      \
                        ElfAddBytes (elf, &num_register, 1);                                                        \
                    }                                                                                               \
                    else                                                                                            \
                    {                                                                                               \
                        printf ("Fucking slave, you are bad programmer!"                                            \
                                "Your math expression is so big, but your another ♂expression♂ is so small...\n");  \
                        return 1;                                                                                   \
                    } 

const size_t VAR_WASNT_CREATED = 1; // Not divisible by 8 - good
const size_t JMP_NUM_SIZE = 4;
const size_t ALIGN = 0x1000;
const char XMM_MAX = 16;

const char FIRST_LOCAL_VAR = 0xF8; // [rbp - 8]
const char FIRST_PARAMETER = 0x10; // [rbp + 10]
const char SIZEOF_ALIGN    = 8;
const char MAX_LOCAL_VAR   = 0x80;
const char MAX_PARAMETER   = 0x78;

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
    char xmm_counter = 0;
};

struct PointerPlace
{
    size_t point = __SIZE_MAX__;
    PointerPlace* next_place = nullptr;
};

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

bool ElfAddLibrary  (Elf* elf, const char* lib_name, FuncPointer* lib_p);

bool ElfWrite       (Elf* elf);

bool FuncPointerConstructor (Elf* elf, const char* func_name);

bool FuncPointerDestructor  (FuncPointer* func_p);

bool FuncPointerGetNewPlace (FuncPointer* func_p, size_t place);

void FuncPointerFillPlaces  (FuncPointer* func_p, char* text);

// ToDo: Write help functions

bool ElfDump (Elf* elf);

bool ElfLog  (Elf* elf);

void GoAsm (Tree* tree);

bool PrintDec  (Elf* elf, element* el);

bool PrintFunc (Elf* elf, element * el);

bool TakeFuncVars (element * el, Stack * vars, char* var_count);

char VarNumber (Stack * vars, const char* var_name);

bool PrintLR    (Elf* elf, element * el, Stack * vars);

bool PrintEqual (Elf* elf, element * el, Stack * vars);

bool PrintArith (Elf* elf, element * el, Stack * vars);

bool PrintCall  (Elf* elf, element * el, Stack * vars);

bool PrintRet   (Elf* elf, element * el, Stack * vars);

bool PrintCond  (Elf* elf, element * el, Stack * vars);

bool PrintComp  (Elf* elf, element * el, Stack * vars);

bool PrintCallParam (Elf* elf, element* el, Stack* vars, int* push_shift);

bool PrintArithNum  (Elf* elf, element* el);

bool PrintArithVar  (Elf* elf, element* el, Stack* vars);

bool PrintArithOper (Elf* elf, element* el, Stack* vars);

bool PrintSaveXMM   (Elf* elf);

bool PrintLoadXMM   (Elf* elf);

bool PrintSqrt (Elf* elf, element* el, Stack* vars);
