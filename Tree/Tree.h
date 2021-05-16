#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

enum Types
{
    NIL   = 0,
    DEC   = 1,
    FUNC  = 2,
    LR    = 3,
    PARAM = 4,
    BODY  = 5,
    ARITH = 6,
    NUM   = 7,
    VAR   = 8,
    IND   = 9,
    RET   = 10,
    IF    = 11,
    WHILE = 12,
    CALL  = 13,
    EQUAL = 14,
    COND  = 15
};

enum LexResult
{
    NOT_THIS = 0,
    SUCCESS  = 1,
    ERROR    = 2
};

struct element
{
    Types type      = NIL;
    element* left   = nullptr;
    element* right  = nullptr;
    char* ind       = nullptr;
    size_t len      = 0;
    char var_pos    = 0;
};

typedef element stk_type;
typedef unsigned long long storm;

/*!
    Bad ded idet v botalku ("v" = "b", "ku" didn`t fit).
*/

const storm STORMY_PETREL = 0xBADDED1DE7BB07A1;

// const size_t MALE_LEN = 3;

// Old code: without ♂

const char    SIN_STR[] = "♂semen♂";
const char    COS_STR[] = "♂cum♂";
const char   SQRT_STR[] = "♂spanking♂";
const char  PRINT_STR[] = "♂cumming♂";
const char   SCAN_STR[] = "♂swallow♂";
const char     IF_STR[] = "♂fantasies♂";
const char  WHILE_STR[] = "♂let's go♂";
const char RETURN_STR[] = "♂next door♂";

//! This is structure of stack.

struct Stack
{
    storm stormy_petrel_begin = STORMY_PETREL;

    stk_type* buffer = nullptr;
    size_t	size = 0,
        capacity = 0,
        min_capacity = 0;
    int status_error = 12; // See stack_errors in Stack.h

    unsigned long long stk_hash = 0;
    unsigned long long buf_hash = 0;

    storm stormy_petrel_end = STORMY_PETREL;
};

struct Tree
{
    element* head = nullptr;
    Stack    stk  = {};
};

void TreeConstructor (Tree* tree);

void TreeDestructor (Tree* tree);

bool ElementConstructor (Tree* tree, Types type, char** ind, size_t len);

void ElementDestructor (element* el);

void CreateGraph (Tree* tree);

void ElementGraph (FILE* graph, element* el);

bool GoTree (Tree* tree, const char* file_path);

bool LexicalAnalyze (Tree* tree, const char* file_path);

bool LexicalParse (Tree* tree);

int CountSize (FILE* file);

size_t ReadTxt (char** text, const char* file_name);

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
