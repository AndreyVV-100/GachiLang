#include "../Tree/Stack.h"

#define print_ass assert (asm_text); \
                  assert (el);       \
                  assert (vars)

#define TryPrint(func, way) if (func (asm_text, el->way, vars)) \
                            {                                   \
                                printf ("Print error in function %s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
                                printf ("It was on point %p in element: %s\n", el, el->ind);             \
                                return 1;                       \
                            }

void GoAsm (Tree* tree, const char* file_path);

bool PrintService (FILE* asm_text, const char* file_path);

void DeleteFuncSpaces (element* el);

bool PrintDec  (FILE* asm_text, element* el);

bool PrintFunc (FILE * asm_text, element * el);

bool TakeFuncVars (element * el, Stack * vars);

bool PrintParam (FILE * asm_text, element * el, Stack * vars);

bool SaveParam  (FILE * asm_text, element * el, Stack * vars);

size_t VarNumber (Stack * vars, const char* var_name);

bool PrintLR    (FILE * asm_text, element * el, Stack * vars);

bool PrintEqual (FILE * asm_text, element * el, Stack * vars);

bool PrintArith (FILE * asm_text, element * el, Stack * vars);

bool PrintCall  (FILE * asm_text, element * el, Stack * vars);

bool PrintRet   (FILE * asm_text, element * el, Stack * vars);

bool PrintCond  (FILE * asm_text, element * el, Stack * vars);

bool PrintComp  (FILE * asm_text, element * el, Stack * vars, size_t cond_number);
