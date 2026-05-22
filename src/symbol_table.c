#include "symbol_table.h"

static const char *StringFromLabel[] = {
  "Void", "Int", "Char", "String", "Built-in", "Function", "Struct"
};

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

// MAIN //

void initHashTable(HashTable* h, HashTable* global, const char* name, int startingAdress) {
    h->functionName = strdup(name);
    h->relativeAddress = startingAdress;
    for (int i = 0; i < TABLE_SIZE; i++) {
        h->table[i].key = NULL;
        h->table[i].state = EMPTY;
    }
    for (int i = 0; i < TABLE_SIZE; i++) {
        h->tableS[i].key = NULL;
        h->tableS[i].state = EMPTY;
    }
    h->parent = global;
    initStructScope(h->structs, startingAdress);
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
    HashEntry entry;

    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        entry = h->table[pos];

        // SI trouvé
        if (entry.state == OCCUPIED 
            && entry.key != NULL
            && strcmp(entry.key, key) == 0
            && !(entry.symbol.typ == SYM_FUNCTION || entry.symbol.typ == SYM_BUILTIN)) {
            return &h->table[pos].symbol;
        }
    }
    return NULL;
}

static Symbol* searchFunction(HashTable *h, const char* key) {
    unsigned int index = hash(key);
    HashEntry entry;

    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        entry = h->table[pos];

        // SI fonction trouvé
        if (entry.state == OCCUPIED 
            && entry.key != NULL
            && strcmp(entry.key, key) == 0
            && (entry.symbol.typ == SYM_FUNCTION || entry.symbol.typ == SYM_BUILTIN)) {
            return &h->table[pos].symbol;
        }
    }

    return NULL;
}

Symbol* lookup(HashTable *h, const char* key) {
    Symbol *s;
    for (HashTable *tmp = h; tmp; tmp = tmp->parent) {
        s = search(tmp, key);
        if (s) {
            return s;
        }
    }
    return NULL;
}

Symbol* lookupFunction(HashTable *h, const char* key) {
    Symbol *s;
    for (HashTable *tmp = h; tmp; tmp = tmp->parent) {
        s = searchFunction(tmp, key);
        if (s) {
            return s;
        }
    }
    return NULL;
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

void printVariablesAndFunctions(HashTable *h) {
    int isEmpty = 1; // Flag checking if the table is empty
    printf("RELATIVE = %-20d\n", h->relativeAddress);

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry *e = &h->table[i];
        switch(e->state) {
            case EMPTY:
            case DELETED:
                break;
            default: {
                Symbol s = e->symbol;

                printf("KEY = %-20s | TYPE = %-8s ", e->key, StringFromLabel[s.typ]);
                printf("| %-6s ", s.isGlobal ? "Global" : "Local");
                printf("| ADDRESS = %-3d ", s.address);

                switch(s.typ) {
                    case SYM_FUNCTION:
                    case SYM_BUILTIN:
                        printf("| RETURN TYPE = %-5s ", StringFromLabel[s.Value.value_funct.returnType]);
                        printf("| NUMBER PARAMS = %-5d", s.Value.value_funct.numberParams);

                        for (int i = 0; i < s.Value.value_funct.numberParams; i++) {
                            printf("%-5s", StringFromLabel[s.Value.value_funct.paramTypes[i]]);
                        }
                        break;
                    default:
                        break;
                }
                printf("\n");
                isEmpty = 0;
                break;
            }
        }
    }
    if (isEmpty) printf("EMPTY VARIABLE TABLE\n");
}

void printStructureVariables(HashTable *h) {
    int isEmpty = 1; // Flag checking if the table is empty

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashSEntry *e = &h->tableS[i];
        switch(e->state) {
            case EMPTY:
            case DELETED:
                break;
            default: {
                SymbolS s = e->symbol;

                printf("KEY = %-20s ", e->key);
                printf("| %-6s ", s.isGlobal ? "Global" : "Local");
                printf("| ADDRESS = %-3d ", s.address);

                printf("| STRUCT = %-20s | TOTAL SIZE = %-5d ", s.structName, s.size);
                printf("\n");
                isEmpty = 0;
                break;
            }
        }
    }
    if (isEmpty) printf("EMPTY STRUCTURE VARIABLE TABLE\n"); 
} 


void freeHashTable(HashTable *h) {
    if (!h) return;

    if (h->functionName) {
        free(h->functionName);
        h->functionName = NULL;
    }

    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (h->table[i].state == OCCUPIED) {
            free(h->table[i].key);
            h->table[i].key = NULL;

            if (h->table[i].symbol.typ == SYM_STRING && h->table[i].symbol.Value.value_str != NULL) {
                free(h->table[i].symbol.Value.value_str);
                h->table[i].symbol.Value.value_str = NULL;
            }

            h->table[i].state = DELETED;
        }
    }
    
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (h->tableS[i].state == OCCUPIED) {
            free(h->tableS[i].key);
            h->tableS[i].key = NULL;

            if (h->tableS[i].symbol.structName) {
                free(h->tableS[i].symbol.structName);
                h->tableS[i].symbol.structName = NULL;
            }

            h->tableS[i].state = DELETED;
        }
    }

    freeStructScope(h->structs);
    free(h);
}

extern void addBuiltIns(HashTable *global) {
    Symbol s = {0};
    s.typ = SYM_BUILTIN;
    s.Value.value_funct.numberParams = 1;
    s.Value.value_funct.returnType = SYM_NONE;
    s.address = -1;
    s.isGlobal = 1;

    // Putchar
    s.Value.value_funct.paramTypes[0] = SYM_CHAR;
    insert(global, "putchar", s);

    // Putint
    s.Value.value_funct.paramTypes[0] = SYM_INT;
    insert(global, "putint", s);

    s.Value.value_funct.paramTypes[0] = SYM_NONE;

    // Getchar
    s.Value.value_funct.numberParams = 0;
    s.Value.value_funct.returnType = SYM_CHAR;
    insert(global, "getchar", s);

    // Getint
    s.Value.value_funct.returnType = SYM_INT;
    insert(global, "getint", s);
}

// STRUCTS //

StructDef* lookupStruct(HashTable *table, const char* name) {
    unsigned int index = hash(name);
    
    for (HashTable *tmp = table; tmp; tmp = tmp->parent) {

        for (int i = 0; i < TABLE_SIZE; i++) {
            unsigned int pos = (index + i) % TABLE_SIZE;
            StructDef *entry = &tmp->structs[pos];
            // SI trouvé

            if (entry->state == EMPTY) {
                break;
            }
            
            if (entry->state == OCCUPIED 
                && entry->structName != NULL
                && strcmp(entry->structName, name) == 0){
                return entry;
            }
        }
    }
    return NULL;
}

int insertStruct(HashTable *table, StructDef st) {
    unsigned int index = hash(st.structName);

    // Sondage linéaire
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        // Place libre
        if (table->structs[pos].state != OCCUPIED) {
            table->structs[pos] = st;
            table->structs[pos].structName = strdup(st.structName);
            table->structs[pos].state = OCCUPIED;

            printf("INSERTED : %s\n", table->structs[pos].structName);
            return 1;
        // Déjà dans la hash map
        } else {
            if (table->structs[pos].structName && strcmp(table->structs[pos].structName, st.structName) == 0) {
                return 0;
            }
        }
    }
    // Table pleine
    return 0;
}

// STRUCT VARIABLES

int insertStructVariable(HashTable *h, const char* key, SymbolS symbol) {
    unsigned int index = hash(key);

    // Sondage linéaire
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        // Place libre
        if (h->tableS[pos].state != OCCUPIED) {
            h->tableS[pos].key = strdup(key);
            h->tableS[pos].symbol = symbol;
            h->tableS[pos].state = OCCUPIED;

            // printf("INSERTED : %s\n", h->table[pos].key);
            return 1;
        // Déjà dans la hash map
        } else {
            if (strcmp(h->tableS[pos].key, key) == 0) {
                return 0;
            }
        }
    }
    // Table pleine
    return 0;
}