#ifndef __SYMBOL_H__
#define __SYMBOL_H__
#include "struct_table.h"

#define TABLE_SIZE 211

// Variables / functions

typedef struct {
    TypeValue returnType;

    int numberParams;
    TypeValue paramTypes[99];
} FunctionInfo;

typedef struct {
    TypeValue typ;
    FunctionInfo value_funct;

    int address;
    int isGlobal;
} Symbol;

typedef struct {
    char* key;
    Symbol symbol;
    EntryState state;
} HashEntry;

// Struct variables

typedef struct {
    char* structName;
    int size;
    int address;
    int isGlobal;
} SymbolS;

typedef struct {
    char* key;
    SymbolS symbol;
    EntryState state;
} HashSEntry;

typedef struct HashTable {
    char* functionName;
    int relativeAddress;
    
    HashEntry table[TABLE_SIZE]; // Variables / functions
    HashSEntry tableS[TABLE_SIZE]; // Struct variables
    StructDef structs[TABLE_SIZE]; // Contains all structs from a scope
    struct HashTable *parent;
} HashTable;

void initHashTable(HashTable* h, HashTable* global, const char* name, int startingAdress);
int insert(HashTable *h, const char* key, Symbol symbol);

Symbol* lookup(HashTable *h, const char* key);
Symbol* lookupFunction(HashTable *h, const char* key);
SymbolS* lookupStructureVariable(HashTable *h, const char* key);
StructDef* lookupField(HashTable *h, const char* name);

int deleteH(HashTable *h, const char* key);
void freeHashTable(HashTable *h);

void printVariablesAndFunctions(HashTable *h);
void printStructureVariables(HashTable *h);

void addBuiltIns(HashTable *global);

StructDef* lookupStruct(HashTable *table, const char* name);
int insertStruct(HashTable *table, StructDef st);

int insertStructVariable(HashTable *h, const char* key, SymbolS symbol);

#endif