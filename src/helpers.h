#ifndef __HELPERS_H__
#define __HELPERS_H__
#include "symbol_table.h"
#include "struct_table.h"
#include "tree.h"

// HELPERS SYMBOL HANDLER //

Symbol makeIntSymbol(int v);
Symbol makeCharSymbol(char c);
int castCheck(TypeValue LValue, TypeValue RValue);
int sizeofType(TypeValue t);
int isGlobalScope(HashTable* h);
void printAllTables(Node *n);

// HELPERS NASM HANDLER //

const char* getReserveDirective(TypeValue t);
int isBooleanExp(Node *node);
int numberArgs(Node *node);
int getLocalStackTable(HashTable *h);
int align16(int n);

// RUNTIME FUNCTIONS HANDLER //

void writeRuntimeBss(FILE *f);
void writeMyGetchar(FILE *f);
void writeMyPutchar(FILE *f);
void writeMyGetint(FILE *f);
void writeMyPutint(FILE *f);

#endif