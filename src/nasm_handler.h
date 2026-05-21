#ifndef __NASM_HANDLER_H__
#define __NASM_HANDLER_H__
#include "tree.h"
#include "symbol_table.h"
#include "struct_table.h"
#include "helpers.h"

void parcoursArbre(Node *node, FILE *f);

#endif