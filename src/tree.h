/* tree.h */

#ifndef __TREE_H__
#define __TREE_H__

#include "symbol_table.h"
#include "struct_table.h"

typedef enum {
  prog,

  declVars, declVar, declVarStruct, 
  declFoncts, declFonct, enTete, declarateurs,
  declStruct, declChamps,
  corps, corpsStruct,
  ListExp, ListTypVar, TypVar, Parametres, Arguments, 
  SuiteInstr,

  Exp, TB, FB, M, E, T,

  assign,
  fieldAccess,

  character, add, sub, mul, divstar, mod, notInstr,
  unaryplus, unaryminus,
  num,
  id,
  typeInt, typeChar, typeVoid, typeStruct,
  orderInf, orderInfEquals, orderSup, orderSupEquals, 
  equals, notEquals,
  orExp, andExp, ifSt, elseSt, whileSt, appelFonct, returnSt, voidSt, structSt
  /* list all other node labels, if any */
  /* The list must coincide with the string array in tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} label_t;

typedef enum {
  VALUE_NONE,
  VALUE_INT,
  VALUE_CHAR,
  VALUE_STRING
} type_value;

typedef struct Node {
  label_t label;

  type_value typ;
  union {
    int val_int;
    char val_char;
    char* val_str;
  } value;

  struct Node *firstChild, *nextSibling;
  int lineno;

  HashTable *symTable;
} Node;

Node *makeNode(label_t label);
Node *makeNodeInt(label_t label, int i);
Node *makeNodeChar(label_t label, char c);
Node *makeNodeString(label_t label, const char* str);
const char *strToLabel(label_t label);

void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling

#endif
