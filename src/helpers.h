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

void write_runtime_bss(FILE *f);
void write_my_getchar(FILE *f);
void write_my_putchar(FILE *f);
void write_my_getint(FILE *f);
void write_my_putint(FILE *f);

#endif