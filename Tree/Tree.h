#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#pragma warning (disable : 4996)

#define create_el element* el_crt = ElementConstructor (stk, type, sy)

#define tree_error_printf {                                                                      \
                          printf ("\n""Ошибка: дерево повреждено. Обратитесь к разработчику.\n");\
                          break;                                                                 \
                          }

#define lex_ass assert (tree);  \
                assert (code);  \
                assert (*code)

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
    VAR   = 8
};

enum LexType
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
};

typedef element stk_type;
typedef unsigned long long storm;

/*!
    Bad ded idet v botalku ("v" = "b", "ku" didn`t fit).
*/

const storm STORMY_PETREL = 0xBADDED1DE7BB07A1;

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

bool LexicalAnalyze (Tree* tree, const char* file_path);

int CountSize (FILE* file);

size_t ReadTxt (char** text, const char* file_name);

void  SkipSpaces (char** eq, size_t* line_now);

bool GetElement (Tree* tree, char** code);

LexType CheckLR    (Tree* tree, char** code);

LexType CheckBody  (Tree* tree, char** code);

LexType CheckParam (Tree* tree, char** code);

LexType CheckArith (Tree* tree, char** code);

LexType CheckInd   (Tree* tree, char** code);

LexType CheckNum   (Tree* tree, char** code);

bool IsMale (const char* code);

/*
Old code:

element* InsertHead   (Tree* tree, Types type, double num, char symb);

element* InsertLeft  (element* el, Types type, double num, char symb);

element* InsertRight (element* el, Types type, double num, char symb);
*/