#ifndef __NASM_HANDLER_H__
#define __NASM_HANDLER_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "symbol_table.h"

void parcoursArbre(Node *node, FILE *f);

#endif