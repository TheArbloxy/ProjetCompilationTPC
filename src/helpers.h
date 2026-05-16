#ifndef __HELPERS_H__
#define __HELPERS_H__
#include "symbol_table.h"
#include "tree.h"

// HELPERS SYMBOL HANDLER //

Symbol makeIntSymbol(int v);
Symbol makeCharSymbol(char c);
int castCheck(TypeValue LValue, TypeValue RValue);
Symbol castSymbol(Symbol LValue, Symbol RValue);
int sizeofType(TypeValue t);
int isGlobalScope(HashTable* h);

// HELPERS NASM HANDLER //

const char* getReserveDirective(TypeValue t);
int isBooleanExp(Node *node);
int numberArgs(Node *node);
int getLocalStackTable(HashTable *h);
int align16(int n);

#endif