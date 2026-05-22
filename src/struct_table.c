#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "struct_table.h"

static const char *StringFromLabel[] = {
  "Void", "Int", "Char", "String", "Built-in", "Function"
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

void initStructScope(StructDef* s, int startingAdress) {
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
    unsigned int index = hash(field.key);

    // Sondage linéaire
    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        // Place libre
        if (def->fields[pos].state != OCCUPIED) {
            def->fields[pos] = field;
            def->fields[pos].key = strdup(field.key);
            def->fields[pos].state = OCCUPIED;

            printf("INSERTED : %s\n", def->fields[pos].key);
            return 1;
        // Déjà dans la hash map
        } else {
            if (def->fields[pos].key && strcmp(def->fields[pos].key, field.key) == 0) {
                return 0;
            }
        }
    }
    // Struct pleine
    return 0;
}

StructEntry* lookupField(StructDef *def, const char* name) {
    unsigned int index = hash(name);
    StructEntry entry;

    for (int i = 0; i < TABLE_SIZE; i++) {
        unsigned int pos = (index + i) % TABLE_SIZE;
        entry = def->fields[pos];

        // SI trouvé
        if (entry.state == OCCUPIED 
            && entry.key != NULL
            && strcmp(entry.key, name) == 0){
            return &def->fields[pos];
        }
    }
    return NULL;
}

static void printStructTable(StructDef *s) {
    int isEmpty = 1; // Flag checking if the table is empty
    printf("STRUCT %-20s\n", s->structName);
    printf("TOTAL SIZE = %-20d\n", s->totalSize);

    for (size_t i = 0; i < TABLE_SIZE; i++) {
        StructEntry *e = &s->fields[i];
        
        switch(e->state) {
            case EMPTY:
            case DELETED:
                break;
            default: {
                SymbolStruct st = e->symbol;

                printf("KEY = %-20s | TYPE = %-8s ", e->key, StringFromLabel[st.typ]);
                printf("| %-6s ", st.isGlobal ? "Global" : "Local");
                printf("| ADDRESS = %-3d ", st.address);

                switch(st.typ) {
                    case SYM_INT:
                        printf("| VALUE = %-3d ", st.Value.value_int);
                        break;
                    case SYM_CHAR:
                        printf("| VALUE = %-3c ", st.Value.value_char);
                        break;
                    case SYM_STRING:
                        printf("| VALUE = %-3s ", st.Value.value_str);
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
    if (isEmpty) printf("EMPTY STRUCTURE");
    printf("\n");
}

void printStructScope(StructDef *s) {
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
    if (!s) return;

    // Free toutes les structures de la scope
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (s[i].state == OCCUPIED) {
            free(s[i].structName);

            // Free tous les champs d'une structure
            for (size_t j = 0; i < TABLE_SIZE; i++) {
                if (s[i].fields[j].state == OCCUPIED) {
                    free(s[i].fields[j].key);

                    if (s[i].fields[j].symbol.typ == SYM_STRING 
                        && s[i].fields[j].symbol.Value.value_str != NULL) {
                        free(s[i].fields[j].symbol.Value.value_str);
                    }

                    if (s[i].fields[j].symbol.structName) {
                        free(s[i].fields[j].symbol.structName);
                    }
                }
            }

            s[i].state = DELETED;
        }
    }
}