#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "symbol_table.h"

static const char *StringFromLabel[] = {
  "Void", "Int", "Char", "String", "Built-in Function", "Function"
  /* list all other node labels, if any */
  /* The list must coincide with the label_t enum in tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

// HELPERS //

static unsigned int hash(const char* str) {
    int k = 612;
    int N = 1008;
    int h = 0;
    size_t len = strlen(str);

    for (size_t i = 0; i < len; i++) {
        h += k * h + str[i];
    }
    
    return (h % (int)pow(2, 30)) % N;
}

extern Symbol makeIntSymbol(int v) {
    /*
    Handles a direct access to an int constant.
    */
    Symbol s = {0};
    s.typ = SYM_INT;
    s.Value.value_int = v;
    s.address = -1;
    s.isGlobal = 0;
    return s;
}

extern Symbol makeCharSymbol(char c) {
    /*
    Handles a direct access to a char constant.
    */
    Symbol s = {0};
    s.typ = SYM_CHAR;
    s.Value.value_char = c;
    s.address = -1;
    s.isGlobal = 0;
    return s;
}

extern int castCheck(TypeValue LValue, TypeValue RValue) {
    /*
    Checks if a cast is allowed (char -> int).
    However, (int -> char) isn't allowed by our compiler.
    */
    return (LValue == RValue) || (LValue == SYM_INT && RValue == SYM_CHAR);
} 

extern Symbol castSymbol(Symbol LValue, Symbol RValue) {
    /*
    Handles a symbol cast.
    */
    Symbol result = {0};
    result.typ = LValue.typ;

    switch (LValue.typ) {
        case SYM_INT:
            // Cast from a char to an int
            if (RValue.typ == SYM_CHAR) {
                result.Value.value_int = (int)RValue.Value.value_char;
            } else {
                result.Value.value_int = RValue.Value.value_int;
            }
            break;
        case SYM_CHAR:
            result.Value.value_char = RValue.Value.value_char;
            break;
        default:
            break;
    }

    // Gets the address and the global variable flag from the src to the result.
    result.address = LValue.address;
    result.isGlobal = LValue.isGlobal;

    return result;
}

extern int sizeofType(TypeValue t) {
    switch (t) {
        case SYM_INT:
            return 4;
        case SYM_CHAR:
            return 1;
        default:
            return 4;
    }
}

extern int isGlobalScope(HashTable* h) {
    return !(h->parent);
}

// MAIN //

void initHashTable(HashTable* h, HashTable* global) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        h->table[i].key = NULL;
        h->table[i].state = EMPTY;
    }
    h->parent = global;
}

int insert(HashTable *h, const char* key, Symbol symbol) {
    unsigned int index = hash(key);

    // Sondage linéaire
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        // Place libre
        if (h->table[pos].state != OCCUPIED) {
            h->table[pos].key = strdup(key);
            h->table[pos].symbol = symbol;
            h->table[pos].state = OCCUPIED;

            // printf("INSERTED : %s\n", h->table[pos].key);
            return 1;
        // Déjà dans la hash map
        } else {
            if (strcmp(h->table[pos].key, key) == 0) {
                return 0;
            }
        }
    }
    // Table pleine
    return 0;
}

Symbol* search(HashTable *h, const char* key) {
    unsigned int index = hash(key);

    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;

        // SI trouvé
        if (h->table[pos].state == OCCUPIED 
            && h->table[pos].key != NULL
            && strcmp(h->table[pos].key, key) == 0) {
            return &h->table[pos].symbol;
        }
    }

    return NULL;
}

Symbol* lookup(HashTable *h, const char* key) {
    for (HashTable *tmp = h; tmp != NULL; tmp = tmp->parent) {
        Symbol *s = search(tmp, key);
        if (s) {
            return s;
        }
    }
    return NULL;
}

static int modify(HashTable *h, const char* key, Symbol newSymbol) {
    unsigned int index = hash(key);

    // Sondage linéaire
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        // SI trouvé
        if (h->table[pos].state == OCCUPIED 
            && h->table[pos].key != NULL
            && strcmp(h->table[pos].key, key) == 0) {
            h->table[pos].symbol = newSymbol;
            return 1;
        }
    }
    // Pas trouvé
    return 0;
}

int lookupModify(HashTable *h, const char* key, Symbol newSymbol) {
    for (HashTable *tmp = h; tmp; tmp = tmp->parent) {
        if (modify(tmp, key, newSymbol)) {
            return 1;
        }
    }
    return 0;
}

int deleteH(HashTable *h, const char* key) {
    unsigned int index = hash(key);

    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + 1) % TABLE_SIZE;

        // SI trouvé
        if (h->table[pos].state == OCCUPIED && strcmp(h->table[pos].key, key) == 0) {
            free(h->table[pos].key);
            if (h->table[pos].symbol.typ == SYM_STRING) {
                free(h->table[pos].symbol.Value.value_str);
            }
            h->table[pos].state = DELETED;
            return 1;
        }
    }

    return 0;
}

void printHashTable(HashTable *h) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry *e = &h->table[i];
        switch(e->state) {
            case EMPTY:
                // printf("EMPTY\n");
                break;
            case DELETED:
                // printf("DELETED\n");
                break;
            default:
                printf("KEY = %s | TYPE = %s ", e->key, StringFromLabel[e->symbol.typ]);

                switch(e->symbol.typ) {
                    case SYM_INT:
                        printf("| VALUE = (%d) ", e->symbol.Value.value_int);
                        break;
                    case SYM_CHAR:
                        printf("| VALUE = (%c) ", e->symbol.Value.value_char);
                        break;
                    case SYM_STRING:
                        printf("| VALUE = (%s) ", e->symbol.Value.value_str);
                        break;
                    default:
                        break;
                }
                printf("| ADDRESS = (%d) ", e->symbol.address);
                printf("| IS GLOBAL = %s", e->symbol.isGlobal ? "true" : "false");
                printf("\n");
                break;
        }
    }
}

void freeHashTable(HashTable *h) {
    if (!h) return;

    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (h->table[i].state == OCCUPIED) {
            free(h->table[i].key);

            if (h->table[i].symbol.typ == SYM_STRING && h->table[i].symbol.Value.value_str != NULL) {
                free(h->table[i].symbol.Value.value_str);
            }

            h->table[i].state = DELETED;
        }
    }
    free(h);
}

extern void addBuiltIns(HashTable *global) {
    Symbol s = {0};
    s.typ = SYM_BUILTIN;
    s.address = -1;
    s.isGlobal = 1;

    insert(global, "putchar", s);
    insert(global, "putint", s);
    insert(global, "getchar", s);
    insert(global, "getint", s);
}