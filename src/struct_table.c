#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "struct_table.h"

static const char *StringFromLabel[] = {
  "Void", "Int", "Char", "String", "Built-in", "Function", "Structure"
};

static unsigned int hash(const char* str) {
    /*
    Hash function.
    */
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

void initStructScope(StructDef* s, int startingAdress) {
    /*
    Initiates a structure scope, containing all structures declaration from a scope.
    */
    for (int i = 0; i < TABLE_SIZE; i++) {
        s[i].structName = NULL;
        s[i].state = EMPTY;
        s[i].totalSize = startingAdress;

        for (int j = 0; j < TABLE_SIZE; j++) {
            s[i].fields[j].key = NULL;
            s[i].fields[j].state = EMPTY;
        }
    }
}

int insertField(StructDef *def, StructEntry field) {
    /*
    Attempts to insert a field StructEntry in a structure definition StructDef.
    Returns 1 if successful, 0 otherwise (duplicate value). 
    */
    unsigned int index = hash(field.key);

    // Search
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        // Free place
        if (def->fields[pos].state != OCCUPIED) {
            def->fields[pos] = field;
            def->fields[pos].key = strdup(field.key);
            def->fields[pos].state = OCCUPIED;

            return 1;
        // Already in the hash map
        } else {
            if (def->fields[pos].key && strcmp(def->fields[pos].key, field.key) == 0) {
                return 0;
            }
        }
    }
    // Full map
    return 0;
}

StructEntry* lookupEntry(StructDef *s, const char* name) {
    /*
    Attempts to search for a field StructEntry in a structure definition StructDef.
    Returns the field, or NULL if not found.
    */
    unsigned int index = hash(name);

    // Search
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        StructEntry *entry = &s->fields[pos];

        // If found
        if (entry->state == OCCUPIED 
            && entry->key != NULL
            && strcmp(entry->key, name) == 0){
            return entry;
        }
    }

    return NULL;
}

static void printStructTable(StructDef *s) {
    /*
    Prints all structure variables from a scope.
    */
    int isEmpty = 1; // Flag checking if the table is empty
    printf("%s STRUCT %-20s\n", s->isGlobal ? "GLOBAL" : "LOCAL", s->structName);
    printf("TOTAL SIZE = %-20d\n", s->totalSize);

    for (size_t i = 0; i < TABLE_SIZE; i++) {
        StructEntry *e = &s->fields[i];
        
        switch(e->state) {
            case EMPTY:
            case DELETED:
                break;
            default: {
                printf("KEY = %-20s | TYPE = %-8s ", e->key, StringFromLabel[e->typ]);
                printf("| ADDRESS = %-3d | SIZE = %-3d ", e->offset, e->size);

                switch(e->typ) {
                    case SYM_STRUCT:
                        printf("| STRUCT = %-10s ", e->structName);
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
    if (isEmpty) printf("EMPTY STRUCTURE\n");
    printf("\n");
}

void printStructScope(StructDef *s) {
    /*
    Prints all declared structures from a scope.
    */
    int isEmpty = 1; // Flag checking if the table is empty
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (s[i].state == OCCUPIED) {
            printStructTable(&s[i]);
            isEmpty = 0;
        }
    }
    if (isEmpty) printf("NO STRUCTURE VARIABLE\n");
}

void freeStructScope(StructDef *s) {
    /*
    Free all declared structures from a scope.
    */
    if (!s) return;

    // Iterates though all declared structures from a scope.
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (s[i].state == OCCUPIED) {
            if (s[i].structName) {
                free(s[i].structName);
                s[i].structName = NULL;
            }

            // Free all the fields of a structure
            for (size_t j = 0; j < TABLE_SIZE; j++) {
                if (s[i].fields[j].state == OCCUPIED) {
                    free(s[i].fields[j].key);
                    s[i].fields[j].key = NULL;

                    if (s[i].fields[j].structName) {
                        free(s[i].fields[j].structName);
                        s[i].fields[j].structName = NULL;
                    }
                    
                }
            }
            

            s[i].state = DELETED;
        }
    }
}