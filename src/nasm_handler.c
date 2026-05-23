#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nasm_handler.h"

static int labelCounter = 0;

static const char *ArgumentsRegs[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};
static const char *ArgumentsRegs2[] = {
    "edi", "esi", "edx", "ecx", "r8d", "r9d"
};

static FieldAccessInfo resolveFieldAccess(Node *node, HashTable *h) {
    /*
    Gets a field info for NASM.
    */
    FieldAccessInfo info = {0};
    Node *base = node->firstChild;

    // Variable de base
    SymbolS *baseStruct = lookupStructureVariable(h, base->value.val_str);
    if (!baseStruct) {
        printf("Structure introuvable\n");
        return info;
    }

    StructDef *currentStruct = lookupField(h, baseStruct->structName);
    if (!currentStruct) {
        printf("Variable de la structure introuvable\n");
        return info;
    }

    // Champs suivants (ex : p.a; ou p.color.r;)
    Node *field = base->nextSibling;
    StructEntry *fieldSymbol = NULL;

    while (field) {
        fieldSymbol = lookupEntry(currentStruct, field->value.val_str);
        if (!fieldSymbol) {
            printf("Champ de la structure introuvable\n");
            return info;
        }

        // Si champ intermédiaire (ex : p.color.r) : doit être une structure
        if (field->nextSibling) {
            if (!lookupStruct(h, fieldSymbol->structName)) {
                printf("Champ imbriquée de la introuvable\n");
                return info;
            }
        }

        field = field->nextSibling;

        info.baseSymbol = fieldSymbol;
        info.totalOffset += fieldSymbol->offset;
        info.finalType = fieldSymbol->typ;
    }

    return info;
}

static void genLoadStructField(Node *node, FILE *f, HashTable *h) {
    /*
    Generates structure field accessing in NASM.
    */
    FieldAccessInfo info = resolveFieldAccess(node, h);

    StructEntry *s = info.baseSymbol;
    if (!s) return;

    // Adresse de base
    if (s->isGlobal) {
        if (info.finalType == SYM_CHAR) {
            fprintf(f, "    movzx eax, byte [%s + %d]\n", s->key, info.totalOffset);
        } else {
            fprintf(f, "    mov eax, dword [%s + %d]\n", s->key, info.totalOffset);
        }
    } else {
        if (info.finalType == SYM_CHAR) {
            fprintf(f, "    movzx eax, byte [rbp - %d + %d]\n", s->offset, info.totalOffset);
        } else {
            fprintf(f, "    mov eax, dword [rbp - %d + %d]\n", s->offset, info.totalOffset);
        }
    }
}

static void genLoadVariable(Node *node, FILE *f, HashTable *h) {
    /*
    Generates variable accessing in NASM.
    */
    if (!node) return;

    Symbol *s = lookup(h, node->value.val_str);
    if (!s) return;

    if (s->isGlobal) {
        fprintf(f, "    mov eax, [%s]\n", node->value.val_str);
    } else {
        fprintf(f, "    mov eax, dword [rbp - %d]\n", s->address);
    }

    fprintf(f, "    push rax\n");
}

static void genStoreStructField(Node *node, FILE *f, HashTable *h) {
    /*
    Generates structure field storing in NASM.
    */
    if (!node) return;

    FieldAccessInfo info = resolveFieldAccess(node, h);

    StructEntry *s = info.baseSymbol;
    if (!s) return;

    // résultat au sommet de la pile
    fprintf(f, "    pop rsi\n");

    // Adresse de base
    if (s->isGlobal) {
        if (info.finalType == SYM_CHAR) {
            fprintf(f, "    mov byte [%s + %d], sil\n", s->key, info.totalOffset);
        } else {
            fprintf(f, "    mov dword [%s + %d], esi\n", s->key, info.totalOffset);
        }
    } else {
        if (info.finalType == SYM_CHAR) {
            fprintf(f, "    mov byte [rbp - %d + %d], sil\n", s->offset, info.totalOffset);
        } else {
            fprintf(f, "    mov dword [rbp - %d + %d], eax\n", s->offset, info.totalOffset);
        }
    }
}

static void genStoreVariable(Node *node, FILE *f, HashTable *h) {
    /*
    Generates variable storing in NASM.
    */
    if (!node) return;

    printf("STORE VARIABLE : %s\n", node->value.val_str ? node->value.val_str : "null");

    Symbol *s = lookup(h, node->value.val_str);
    if (!s) return;

    // résultat au sommet de la pile
    fprintf(f, "    pop rsi\n");

    if (s->isGlobal) {
        fprintf(f, "    mov [%s], esi\n", node->value.val_str);
    } else {
        fprintf(f, "    mov dword [rbp - %d], esi\n", s->address);
    }
}

static void genExp(Node *node, FILE *f, HashTable *h) {
    /*
    Generates expression handling in NASM.
    */
    if (!node) return;

    switch (node->label) {
        // constante (int)
        case id:
            switch(node->typ) {
                // int
                case VALUE_INT:
                    fprintf(f, "    push %d\n", node->value.val_int);
                    break;
                default:
                    break;
            }
            break;

        // constante (char)
        case character:
            fprintf(f, "    push %d\n", node->value.val_char);
            break;

        // cas des accès à une variable
        case fieldAccess: {
            if (numberArgs(node) > 1) {
                genLoadStructField(node, f, h);
            } else {
                Node *idNode = node->firstChild;
                genLoadVariable(idNode, f, h);
            }
            break;
        }

        // Opérations binaires
        case Exp:
        case TB:
        case FB:
        case M:
        case E:
        case T: {
            Node *left = node->firstChild;
            Node *op   = left ? left->nextSibling : NULL;
            Node *right= op ? op->nextSibling : NULL;

            genExp(left, f, h);
            genExp(right, f, h);

            fprintf(f, "    pop rbx\n"); // droite
            fprintf(f, "    pop rax\n"); // gauche

            switch (op->label) {
                case add: 
                    fprintf(f, "    add rax, rbx\n");
                    fprintf(f, "    push rax\n");
                    break;
                case sub:
                    fprintf(f, "    sub rax, rbx\n");
                    fprintf(f, "    push rax\n");
                    break;
                case mul:
                    fprintf(f, "    imul rax, rbx\n");
                    fprintf(f, "    push rax\n");
                    break;
                case divstar:
                    fprintf(f, "    cqo\n");
                    fprintf(f, "    idiv rbx\n");
                    fprintf(f, "    push rax\n");
                    break;
                case mod:
                    fprintf(f, "    cqo\n");
                    fprintf(f, "    idiv rbx\n");
                    fprintf(f, "    push rdx\n");
                    break;
                default:
                    printf("Operateur inconnu\n");
                    break;
            }
            break;
        }

        // Appel fonction
        case appelFonct: {
            Node *functionName = node->firstChild;
            Node *args = functionName->nextSibling;

            if (!functionName || !args) return;

            // Arguments
            int argc = numberArgs(args);
            if (argc > 6) {
                printf("Fonctions avec plus de 6 arguments non supportés pour l'instant\n");
                return;
            }

            // Évalutation des arguments
            for (Node *arg = args->firstChild; arg; arg = arg->nextSibling) {
                genExp(arg, f, h);
            }

            // Les placer dans les registres
            for (int i = argc - 1; i >= 0; i--) {
                fprintf(f, "    pop %s\n", ArgumentsRegs[i]);
            }

            // Fonctions builtin (getchar et getint)
            if (strcmp(functionName->value.val_str, "getchar") == 0) {
                fprintf(f, "    call my_getchar\n");
            } else if (strcmp(functionName->value.val_str, "getint") == 0) {
                fprintf(f, "    call my_getint\n");
            // Fonctions normales
            } else {
                fprintf(f, "    call _%s\n", functionName->value.val_str);
            }

            fprintf(f, "    push rax\n");
            break;
        }

        default:
            // fallback
            for (Node *child = node->firstChild; child; child = child->nextSibling) {
                genExp(child, f, h);
            }
            break;
    }
}

static void genCond(Node *node, FILE *f, HashTable *h, char *trueLabel, char *falseLabel) {
    /*
    Generates comparaison expressions in NASM.
    */
    if (!node) return;

    switch (node->label) {
        case notInstr: { /* ! */
            Node *expr = node ->firstChild;
            genCond(expr, f, h, falseLabel, trueLabel);
            break;
        }
        case Exp: { /* Or */
            Node *LValue = node->firstChild;
            Node *op = LValue->nextSibling;
            Node *RValue = op->nextSibling;

            int labelEnd = labelCounter++;

            char midLabel[64];
            sprintf(midLabel, ".Lor_right_%d", labelEnd);

            genCond(LValue, f, h, trueLabel, midLabel);
            fprintf(f, "%s:\n", midLabel); // RValue
            genCond(RValue, f, h, trueLabel, falseLabel);
            break;
        }
        case TB: { /* And*/
            Node *LValue = node->firstChild;
            Node *op = LValue->nextSibling;
            Node *RValue = op->nextSibling;

            int labelEnd = labelCounter++;

            char midLabel[64];
            sprintf(midLabel, ".Land_right_%d", labelEnd);

            genCond(LValue, f, h,  midLabel, falseLabel);
            fprintf(f, "%s:\n", midLabel); // RValue
            genCond(RValue, f, h,  trueLabel, falseLabel);
            break;
        }
        case M: /* <, <=, >, >= */
        case FB: { /* ==, != */
            Node *LValue = node->firstChild;
            Node *op = LValue->nextSibling;
            Node *RValue = op->nextSibling;

            genExp(LValue, f, h);
            genExp(RValue, f, h);

            fprintf(f, "    pop rbx\n"); // RValue
            fprintf(f, "    pop rax\n"); // LValue
            fprintf(f, "    cmp rax, rbx\n");

            switch (op->label) {
                case orderInf: {
                    fprintf(f, "    jl %s\n", trueLabel);
                    break;
                }
                case orderInfEquals: {
                    fprintf(f, "    jle %s\n", trueLabel);
                    break;
                }
                case orderSup: {
                    fprintf(f, "    jg %s\n", trueLabel);
                    break;
                }
                case orderSupEquals: {
                    fprintf(f, "    jge %s\n", trueLabel);
                    break;
                }
                case equals: {
                    fprintf(f, "    je %s\n", trueLabel);
                    break;
                }
                case notEquals: {
                    fprintf(f, "    jne %s\n", trueLabel);
                    break;
                }
                default : {
                    printf("Opérateur inconnu.\n");
                    return;
                }
            }
            // Si faux
            fprintf(f, "    jmp %s\n", falseLabel);
            break;
        }
        default : {
            genExp(node, f, h);
            fprintf(f, "    pop rax\n");
            fprintf(f, "    cmp rax, 0\n");

            fprintf(f, "    jne %s\n", trueLabel);

            fprintf(f, "    jmp %s\n", falseLabel);
            break;
            
        }
    }

}

static void genBoolExp(Node *node, FILE *f, HashTable *h) {
    /*
    Generates a boolean expression (0/1) in NASM.
    */
    if (!node) return;

    int labelEnd = labelCounter++;

    char trueLabel[64], falseLabel[64], endLabel[64];
    sprintf(trueLabel, ".Lbool_true_%d", labelEnd);
    sprintf(falseLabel, ".Lbool_false_%d", labelEnd);
    sprintf(endLabel, ".Lbool_end_%d", labelEnd);

    // Test de comparaison
    genCond(node, f, h, trueLabel, falseLabel);

    // Si true
    fprintf(f, "%s:\n", trueLabel);
    fprintf(f, "    push 1\n");
    fprintf(f, "    jmp %s\n", endLabel);
    
    // Sinon
    fprintf(f, "%s:\n", falseLabel);
    fprintf(f, "    push 0\n");
    fprintf(f, "%s:\n", endLabel);
}

static void genAssign(Node *node, FILE *f, HashTable *h) {
    /*
    Generates assigns in NASM.
    */
    if (!node) return;

    Node *var = node->firstChild;
    Node *expr = var->nextSibling;

    // printf("LABEL VAR : %s\n", var ? strToLabel(var->label) : "null");
    // printf("LABEL EXPR : %s\n", expr ? strToLabel(expr->label) : "null");
    
    /* Expression */
    if (isBooleanExp(expr)) {
        genBoolExp(expr, f, h);
    } else {
        genExp(expr, f, h);
    }

    if (numberArgs(var) > 1) {
        genStoreStructField(var, f, h);
    } else {
        genStoreVariable(var->firstChild, f, h);
    }
}

static void genFunctCall(Node *node, FILE *f, HashTable *h) {
    /*
    Generates function calls in NASM.
    */
    Node *functionName = node->firstChild;
    Node *args = functionName->nextSibling;

    if (!functionName) return;

    // Générer arguments
    if (args && args->firstChild) {
        Node *arg = args->firstChild;
        genExp(arg, f, h);
        fprintf(f, "    pop rdi\n");
    }

    // Fonctions builtin (putchar et putint)
    if (strcmp(functionName->value.val_str, "putchar") == 0) {
        fprintf(f, "    call my_putchar\n");
    } else if (strcmp(functionName->value.val_str, "putint") == 0) {
        fprintf(f, "    call my_putint\n");
    // Fonctions normales
    } else {
        fprintf(f, "    call _%s\n", functionName->value.val_str);
    }
}

static void genFunctBeginning(Node *node, FILE *f) {
    /*
    Generates the beginning of a function in NASM.
    */
    if (!node) return;

    fprintf(f, "    push rbp\n");
    fprintf(f, "    mov rbp, rsp\n");

    int size = align16(node->symTable->relativeAddress);
    if (size > 0) {
        fprintf(f, "    sub rsp, %d\n", size);
    }
}

static void genFunctEnd(FILE *f) {
    /*
    Generates the end of a function in NASM.
    */
    fprintf(f, "    mov rsp, rbp\n");
    fprintf(f, "    pop rbp\n");
    fprintf(f, "    ret\n");
}

static void genReturn(Node *node, FILE *f, HashTable *h) {
    /*
    Generates a return instruction in NASM.
    */
    if (!node) return;

    Node *expr = node->firstChild;
    if (!expr) return;

    /* Expression */
    if (isBooleanExp(expr)) {
        genBoolExp(expr, f, h);
    } else {
        genExp(expr, f, h);
    }

    fprintf(f, "    pop rax\n");

    genFunctEnd(f);
}

static void genReturnVoid(FILE *f) {
    /*
    Generates a void return instruction in NASM.
    */
    fprintf(f, "    ret\n");
}

static void genInstr(Node *node, FILE *f, HashTable *h) {
    /*
    Generates instructions in NASM.
    */
    if (!node) return;

    for (Node *child = node->firstChild; child; child = child->nextSibling) {
        // printf("LABEL CHILD : %s\n", child ? strToLabel(child->label) : "null");

        switch (child->label) {
            case assign: {
                genAssign(child, f, h);
                break;
            }
            case appelFonct: {
                genFunctCall(child, f, h);
                break;
            }
            case ifSt: {
                Node *cond = child->firstChild;
                Node *thenInstr = cond->nextSibling;
                Node *elseInstr = thenInstr->nextSibling;

                int labelEnd = labelCounter++;

                char trueLabel[64], falseLabel[64], endLabel[64];
                sprintf(trueLabel, ".Lifelse_true_%d", labelEnd);
                sprintf(falseLabel, ".Lifelse_false_%d", labelEnd);
                sprintf(endLabel, ".Lifelse_end_%d", labelEnd);

                // Test de comparaison
                genCond(cond, f, h, trueLabel, falseLabel);

                // Si true
                fprintf(f, "%s:\n", trueLabel);
                genInstr(thenInstr, f, h);
                fprintf(f, "    jmp %s\n", endLabel);
                
                // Sinon, on fait un jump après
                fprintf(f, "%s:\n", falseLabel);
                genInstr(elseInstr, f, h);
                fprintf(f, "%s:\n", endLabel);
                break;
            }
            case elseSt: {
                Node *cond = child->firstChild;
                Node *thenInstr = cond->nextSibling;
                Node *elseInstr = thenInstr->nextSibling;

                int labelEnd = labelCounter++;

                char trueLabel[64], falseLabel[64], endLabel[64];
                sprintf(trueLabel, ".Lifelse_true_%d", labelEnd);
                sprintf(falseLabel, ".Lifelse_false_%d", labelEnd);
                sprintf(endLabel, ".Lifelse_end_%d", labelEnd);

                // Test de comparaison
                genCond(cond, f, h, trueLabel, falseLabel);

                // Si true
                fprintf(f, "%s:\n", trueLabel);
                genInstr(thenInstr, f, h);
                fprintf(f, "    jmp %s\n", endLabel);
                
                // Sinon, on fait un jump après
                fprintf(f, "%s:\n", falseLabel);
                genInstr(elseInstr, f, h);
                fprintf(f, "%s:\n", endLabel);
                break;
            }
            case returnSt: {
                genReturn(child, f, h);
                break;
            }
            case voidSt: {
                genReturnVoid(f);
                break;
            }
            default:
                break;
        }
    }
}

static void genGlobalVariables(HashTable* h, FILE *f) {
    /*
    Generates global variables handling in NASM/
    */
    if (!h) return;

    fprintf(f, "section .bss\n");
    writeRuntimeBss(f);

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry *entry = &h->table[i]; // Variables & fonctions
        if (entry->state == OCCUPIED) {
            Symbol *s = &entry->symbol;

            // ignorer fonctions
            switch (s->typ) {
                case SYM_BUILTIN:
                case SYM_FUNCTION:
                case SYM_NONE:
                case SYM_STRING:
                    break;
                case SYM_INT:
                case SYM_CHAR:
                    fprintf(f, "    %s: %s 1\n", entry->key, getReserveDirective(s->typ));
                case SYM_STRUCT:
                    break;
                default:
                    break;
            }
        }

        HashSEntry *entry2 = &h->tableS[i]; // Variables et structures
        if (entry2->state == OCCUPIED) {
            SymbolS *s = &entry2->symbol;
            fprintf(f, "    %s: resb %d\n", entry2->key, s->size);
        }
    }

    fprintf(f, "\n");
}

static void genParametreStockage(Node *node, FILE *f, HashTable *h) {
    /*
    Generates function's parameters handling in NASM.
    */
    if (!node) return;

    int i = 0;
    Node *p = node->firstChild;
    for (Node *c = p->firstChild; c; c = c->nextSibling) {
        Node *ident = c->firstChild->nextSibling;
        if (ident->label == id) {
            Symbol *s = lookup(h, ident->value.val_str);
            if (!s) return;

            fprintf(f, "    mov dword [rbp - %d], %s\n", s->address, ArgumentsRegs2[i]);
        }

        i++;

        if (i >= 6) {
            printf("Fonctions avec plus de 6 arguments non supportés pour l'instant\n");
            return;
        }
    }

}

static int isMainFunction(Node *node) {
    /*
    Checks if the node is the main function.
    */
    Node *header = node->firstChild;
    if (!header) return 0;

    for (Node *child = header->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == id &&
            child->typ == VALUE_STRING &&
            strcmp(child->value.val_str, "main") == 0) {
            return 1;
        }
    }
    return 0;
}

extern void parcoursArbre(Node *n, FILE *f) {
    /*
    Start of the NASM compile progress.
    */

    if (!n) return;
    // printf("LABEL N : %s\n", n ? strToLabel(n->label) : "null");

    Node *declVars      = n->firstChild;
    Node *declFunctions = declVars->nextSibling;

    switch (n->label) {
        // Global variables
        case prog:
            genGlobalVariables(n->symTable, f);
            
            fprintf(f, "global _start\n\nsection .text\n");;
            fprintf(f, "\n_start:\n");
            fprintf(f, "    call _main\n");

            fprintf(f, "    mov rdi, rax\n    mov rax, 60\n    syscall\n");
        // Functions
        case declFoncts:
            for (Node *declFonct = declFunctions->firstChild; declFonct; declFonct = declFonct->nextSibling) {
                Node *enTete = declFonct->firstChild;
                Node *corps = enTete->nextSibling;
                Node *params = enTete->firstChild->nextSibling->nextSibling;

                // Label fonction
                if (declFonct->symTable) fprintf(f, "_%s:\n", declFonct->symTable->functionName);
                genFunctBeginning(declFonct, f);

                // Paramètres
                genParametreStockage(params, f, declFonct->symTable);

                genInstr(corps->firstChild->nextSibling, f, declFonct->symTable);
            }
        default:
            break;
    }

    writeMyGetchar(f);
    writeMyPutchar(f);
    writeMyGetint(f);
    writeMyPutint(f);
}