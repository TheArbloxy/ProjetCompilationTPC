#ifndef __SHANDLER_H__
#define __SHANDLER_H__
#include "tree.h"
#include "symbol_table.h"
#include "struct_table.h"
#include "helpers.h"

int buildSymbolTables(Node *n, HashTable *table);

#endif