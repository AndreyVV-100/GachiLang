#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Stack.h"

const char    SIN_STR[] = "♂semen♂";
const char    COS_STR[] = "♂cum♂";
const char   SQRT_STR[] = "♂spanking♂";
const char  PRINT_STR[] = "♂cumming♂";
const char   SCAN_STR[] = "♂swallow♂";
const char     IF_STR[] = "♂fantasies♂";
const char  WHILE_STR[] = "♂let's go♂";
const char RETURN_STR[] = "♂next door♂";

extern element* PARSE_ERR;

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

int CountSize (FILE* file);

size_t ReadTxt (char** text, const char* file_name);

void SaveTree (Tree* tree);

void SaveElem (element* el, FILE* tree_file, int tab);

bool LoadTree (Tree* tree);

element* LoadElem (Tree* tree, char** pos_now);
