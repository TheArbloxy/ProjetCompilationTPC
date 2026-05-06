#ifndef __SYMBOL_H__
#define __SYMBOL_H__
#include <stdio.h>

#define TABLE_SIZE 211

typedef enum {
    SYM_NONE,
    SYM_INT,
    SYM_CHAR,
    SYM_STRING
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
} Symbol;

typedef struct {
    char* key;
    Symbol symbol;
    EntryState state;
} HashEntry;

typedef struct HashTable {
    HashEntry table[TABLE_SIZE];
    struct HashTable *parent;
} HashTable;

void initHashTable(HashTable* h, HashTable* global);
int insert(HashTable *h, const char* key, Symbol symbol);
Symbol* lookup(HashTable *h, const char* key);
Symbol* search(HashTable *h, const char* key);
int lookupModify(HashTable *h, const char* key, Symbol* newSymbol);
int deleteH(HashTable *h, const char* key);
void freeHashTable(HashTable *h);
void printHashTable(HashTable *h);

#endif