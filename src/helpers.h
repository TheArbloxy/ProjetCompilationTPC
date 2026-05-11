#ifndef __HELPERS_H__
#define __HELPERS_H__
#include "symbol_table.h"
#include "tree.h"

Symbol makeIntSymbol(int v);
Symbol makeCharSymbol(char c);
int castCheck(TypeValue LValue, TypeValue RValue);
Symbol castSymbol(Symbol LValue, Symbol RValue);
int sizeofType(TypeValue t);
int isGlobalScope(HashTable* h);
const char* getReserveDirective(TypeValue t);

#endif