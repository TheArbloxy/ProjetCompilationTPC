/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
extern int yylineno;       /* from lexer */

static const char *StringFromLabel[] = {
  "prog",

  "declVars", "declVar", "declVarStruct", 
  "declFoncts", "declFonct", "enTete", "declarateurs",
  "declStruct", "declChamps",
  "corps", "corpsStruct",
  "listExp", "listTypVar", "typVar", "parametres", "arguments", 
  "suiteInstr",

  "Exp", "TB", "FB", "M", "E", "T",

  "assign",
  "access",

  "character", "plus", "moins", "mul", "divstar", "mod", "not",
  "unaireplus", "unairemoins",
  "num",
  "ident",
  "typeInt", "typeChar", "typeVoid", "typeStruct",
  "orderInf", "orderInfEquals", "orderSup", "orderSupEquals", 
  "equals", "notEquals",
  "or", "and", "if", "ifElse", "while", "appelFonct", "return", "void", "struct"
  /* list all other node labels, if any */
  /* The list must coincide with the label_t enum in tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

Node *makeNode(label_t label) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    printf("Run out of memory\n");
    exit(1);
  }
  node->label = label;
  node->typ = VALUE_NONE;
  node-> firstChild = node->nextSibling = NULL;
  node->lineno=yylineno;

  node->symTable = NULL;
  return node;
}

Node *makeNodeInt(label_t label, int i) {
  Node *node = makeNode(label);
  node->typ = VALUE_INT;
  node->value.val_int = i;
  return node;
}

Node *makeNodeChar(label_t label, char c) {
  Node *node = makeNode(label);
  node->typ = VALUE_CHAR;
  node->value.val_char = c;
  return node;
}

Node *makeNodeString(label_t label, const char* str) {
  Node *node = makeNode(label);
  node->typ = VALUE_STRING;
  node->value.val_str = strdup(str);
  return node;
}

void addSibling(Node *node, Node *sibling) {
  Node *curr = node;
  while (curr->nextSibling != NULL) {
    curr = curr->nextSibling;
  }
  curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
  if (parent->firstChild == NULL) {
    parent->firstChild = child;
  }
  else {
    addSibling(parent->firstChild, child);
  }
}

void deleteTree(Node *node) {
  if (node->firstChild) {
    deleteTree(node->firstChild);
  }
  if (node->nextSibling) {
    deleteTree(node->nextSibling);
  }

  if (node->typ == VALUE_STRING) {
    free(node->value.val_str);
  }

  if (node->symTable != NULL) {
    freeHashTable(node->symTable);
    node->symTable = NULL;
  }
  
  free(node);
}

const char *strToLabel(label_t label) {
  return StringFromLabel[label];
}

void printTree(Node *node) {
  // printf("Create node %p label %d (%s)\n", node, node->label, StringFromLabel[node->label]);
  static bool rightmost[128]; // tells if node is rightmost sibling
  static int depth = 0;       // depth of current node
  for (int i = 1; i < depth; i++) { // 2502 = vertical line
    printf(rightmost[i] ? "    " : "\u2502   ");
  }
  if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
    printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
  }
  printf("%s", StringFromLabel[node->label]);
  switch(node->typ) {
      case VALUE_INT:
          printf(" (%d)", node->value.val_int);
          break;
      case VALUE_CHAR:
          printf(" (%c)", node->value.val_char);
          break;
      case VALUE_STRING:
          printf(" (%s)", node->value.val_str);
          break;
      default:
          break;
  }
  printf("\n");
  depth++;
  for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
    rightmost[depth] = (child->nextSibling) ? false : true;
    printTree(child);
  }
  depth--;
}
