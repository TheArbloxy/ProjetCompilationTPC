#ifndef __SHANDLER_H__
#define __SHANDLER_H__
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "symbol_table.h"
#include "helpers.h"

void buildSymbolTables(Node *n, HashTable *table);
void printAllTables(Node *n);

#endif