#ifndef __STRUCT_H__
#define __STRUCT_H__
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TABLE_SIZE 211

typedef enum {
    SYM_NONE,
    SYM_INT,
    SYM_CHAR,
    SYM_STRING,
    SYM_BUILTIN,
    SYM_FUNCTION,
    SYM_STRUCT
} TypeValue;

typedef enum {
    EMPTY,
    OCCUPIED,
    DELETED
} EntryState;

typedef struct {
    TypeValue typ;

    union {
        int value_int;
        char value_char;
        char* value_str;
    } Value;

    char* structName;
    int isGlobal;
} SymbolStruct;

typedef struct {
    char* key;
    int offset;
    int size;
    SymbolStruct symbol;
    EntryState state;
} StructEntry;

typedef struct {
    char* structName;
    int totalSize;

    StructEntry fields[TABLE_SIZE];
    EntryState state;
} StructDef; // Contains a struct, with all its fields

void initStructScope(StructDef* s, int startingAdress);
int insertField(StructDef *def, StructEntry field);
StructEntry* lookupField(StructDef *def, const char* name);
void printStructScope(StructDef *s);
void freeStructScope(StructDef *s);

#endif