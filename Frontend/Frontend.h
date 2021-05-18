#include "../TreeBase/Tree.h"

#define create_el element* el_crt = ElementConstructor (stk, type, sy)

#define tree_error_printf {                                                                           \
                          printf ("\n""Error: The tree is damaged. Please contact the developer. \n");\
                          break;                                                                      \
                          }

#define anal_ass assert (tree);  \
                 assert (code);  \
                 assert (*code)

#define parse_ass assert (el_now); \
                  assert (*el_now)

#define check_parse(el) if (el == PARSE_ERR) return PARSE_ERR

#define require_exit  {                                                                                      \
                          printf ("Require error in function %s, line %d\n", __PRETTY_FUNCTION__, __LINE__); \
                          printf ("It was in lexeme: %s\n", (*el_now)->ind);                                 \
                          return PARSE_ERR;                                                                  \
                      }

#define require(what) if ((*el_now)->type != what) \
                          require_exit

#define require_ind   if ((*el_now)->type != IND || (*el_now)->ind == nullptr) \
                          require_exit

#define next *el_now += 1

enum LexResult
{
    NOT_THIS = 0,
    SUCCESS  = 1,
    ERROR    = 2
};

// const size_t MALE_LEN = 3;

// Old code: without ♂

//! This is structure of stack.

bool GoTree (Tree* tree, const char* file_path);

bool LexicalAnalyze (Tree* tree, const char* file_path);

bool LexicalParse (Tree* tree);

void  SkipSpaces (char** eq, size_t* line_now);

bool CheckElement (Tree* tree, char** code);

LexResult CheckLR    (Tree* tree, char** code);

LexResult CheckBody  (Tree* tree, char** code);

LexResult CheckParam (Tree* tree, char** code);

LexResult CheckArith (Tree* tree, char** code);

LexResult CheckInd   (Tree* tree, char** code);

LexResult CheckNum   (Tree* tree, char** code);

bool IsMale (const char* code);

element* GetFunc      (element** el_now);

element* GetFuncParam (element** el_now);

element* GetBody      (element** el_now);

element* GetOper      (element** el_now);

element* GetArith     (element** el_now);

element* GetCall      (element** el_now);

element* GetCallParam (element** el_now);

element* GetReturn    (element** el_now);

element* GetCond      (element** el_now);

element* GetE         (element** el_now);

element* GetT         (element** el_now);

element* GetDegree    (element** el_now);

element* GetUnary     (element** el_now);

element* GetP         (element** el_now);
