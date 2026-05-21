#ifndef __SYMBOL_H__
#define __SYMBOL_H__
#include "struct_table.h"

#define TABLE_SIZE 211

typedef struct {
    TypeValue returnType;

    int numberParams;
    TypeValue paramTypes[99];
} FunctionInfo;

typedef struct {
    TypeValue typ;

    union {
        int value_int;
        char value_char;
        char* value_str;
        FunctionInfo value_funct;
    } Value;

    char* structName;
    int size;

    int address;
    int isGlobal;
} Symbol;

typedef struct {
    char* key;
    Symbol symbol;
    EntryState state;
} HashEntry;

typedef struct HashTable {
    char* functionName;
    int relativeAddress;
    
    HashEntry table[TABLE_SIZE];
    StructDef structs[TABLE_SIZE]; // Contains all structs from a scope
    struct HashTable *parent;
} HashTable;

void initHashTable(HashTable* h, HashTable* global, const char* name, int startingAdress);
int insert(HashTable *h, const char* key, Symbol symbol);

Symbol* lookup(HashTable *h, const char* key);
Symbol* lookupFunction(HashTable *h, const char* key);
Symbol* search(HashTable *h, const char* key);

int lookupModify(HashTable *h, const char* key, Symbol newSymbol);

int deleteH(HashTable *h, const char* key);
void freeHashTable(HashTable *h);

void printHashTable(HashTable *h);
void addBuiltIns(HashTable *global);

StructDef* lookupStruct(HashTable *table, const char* name);

#endif