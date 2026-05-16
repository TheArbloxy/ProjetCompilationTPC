#include "nasm_handler.h"

static int labelCounter = 0;

static const char *ArgumentsRegs[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};
static const char *ArgumentsRegs2[] = {
    "edi", "esi", "edx", "ecx", "r8d", "r9d"
};

void genExp(Node *node, FILE *f) {
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
            Node *idNode = node->firstChild;

            if (!idNode) {
                printf("Erreur : fieldAccess vide\n");
                return;
            }

            fprintf(f, "    mov eax, [%s]\n", idNode->value.val_str);
            fprintf(f, "    push rax\n");
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

            genExp(left, f);
            genExp(right, f);

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
                genExp(arg, f);
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
                genExp(child, f);
            }
            break;
    }
}

void genCond(Node *node, FILE *f, char *trueLabel, char *falseLabel) {
    /*
    Generates comparaison expressions in NASM.
    */
    if (!node) return;

    switch (node->label) {
        case notInstr: { /* ! */
            Node *expr = node ->firstChild;
            genCond(expr, f, falseLabel, trueLabel);
            break;
        }
        case Exp: { /* Or */
            Node *LValue = node->firstChild;
            Node *op = LValue->nextSibling;
            Node *RValue = op->nextSibling;

            int labelEnd = labelCounter++;

            char midLabel[64];
            sprintf(midLabel, ".Lor_right_%d", labelEnd);

            genCond(LValue, f, trueLabel, midLabel);
            fprintf(f, "%s:\n", midLabel); // RValue
            genCond(RValue, f, trueLabel, falseLabel);
            break;
        }
        case TB: { /* And*/
            Node *LValue = node->firstChild;
            Node *op = LValue->nextSibling;
            Node *RValue = op->nextSibling;

            int labelEnd = labelCounter++;

            char midLabel[64];
            sprintf(midLabel, ".Land_right_%d", labelEnd);

            genCond(LValue, f, midLabel, falseLabel);
            fprintf(f, "%s:\n", midLabel); // RValue
            genCond(RValue, f, trueLabel, falseLabel);
            break;
        }
        case M: /* <, <=, >, >= */
        case FB: { /* ==, != */
            Node *LValue = node->firstChild;
            Node *op = LValue->nextSibling;
            Node *RValue = op->nextSibling;

            genExp(LValue, f);
            genExp(RValue, f);

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
            genExp(node, f);
            fprintf(f, "    pop rax\n");
            fprintf(f, "    cmp rax, 0\n");

            fprintf(f, "    jne %s\n", trueLabel);

            fprintf(f, "    jmp %s\n", falseLabel);
            break;
            
        }
    }

}

void genBoolExp(Node *node, FILE *f) {
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
    genCond(node, f, trueLabel, falseLabel);

    // Si true
    fprintf(f, "%s:\n", trueLabel);
    fprintf(f, "    push 1\n");
    fprintf(f, "    jmp %s\n", endLabel);
    
    // Sinon
    fprintf(f, "%s:\n", falseLabel);
    fprintf(f, "    push 0\n");
    fprintf(f, "%s:\n", endLabel);
}

void genAssign(Node *node, FILE *f) {
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
        genBoolExp(expr, f);
    } else {
        genExp(expr, f);
    }

    // résultat au sommet de la pile
    fprintf(f, "    pop rsi\n");

    if (var->label == fieldAccess) {
        Node *idNode = var->firstChild;
        fprintf(f, "    mov [%s], esi\n", idNode->value.val_str);
    } 
}

void genFunctCall(Node *node, FILE *f) {
    /*
    Generates function calls in NASM.
    */
    Node *functionName = node->firstChild;
    Node *args = functionName->nextSibling;

    if (!functionName) return;

    // Générer arguments
    if (args && args->firstChild) {
        Node *arg = args->firstChild;
        genExp(arg, f);
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

void genReturn(Node *node, FILE *f) {
    /*
    Generates a return instruction in NASM.
    */
    if (!node) return;

    Node *expr = node->firstChild;
    if (!expr) return;

    /* Expression */
    if (isBooleanExp(expr)) {
        genBoolExp(expr, f);
    } else {
        genExp(expr, f);
    }

    fprintf(f, "    pop rax\n");
    fprintf(f, "    ret\n");
}

void genReturnVoid(FILE *f) {
    /*
    Generates a void return instruction in NASM.
    */
    fprintf(f, "    ret\n");
}

void genInstr(Node *node, FILE *f) {
    /*
    Generates instructions in NASM.
    */
    if (!node) return;

    for (Node *child = node->firstChild; child; child = child->nextSibling) {
        // printf("LABEL CHILD : %s\n", child ? strToLabel(child->label) : "null");

        switch (child->label) {
            case assign: {
                genAssign(child, f);
                break;
            }
            case appelFonct: {
                genFunctCall(child, f);
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
                genCond(cond, f, trueLabel, falseLabel);

                // Si true
                fprintf(f, "%s:\n", trueLabel);
                genInstr(thenInstr, f);
                fprintf(f, "    jmp %s\n", endLabel);
                
                // Sinon, on fait un jump après
                fprintf(f, "%s:\n", falseLabel);
                genInstr(elseInstr, f);
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
                genCond(cond, f, trueLabel, falseLabel);

                // Si true
                fprintf(f, "%s:\n", trueLabel);
                genInstr(thenInstr, f);
                fprintf(f, "    jmp %s\n", endLabel);
                
                // Sinon, on fait un jump après
                fprintf(f, "%s:\n", falseLabel);
                genInstr(elseInstr, f);
                fprintf(f, "%s:\n", endLabel);
                break;
            }
            case returnSt: {
                genReturn(child, f);
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

void genGlobalVariables(HashTable* h, FILE *f) {
    /*
    Generates global variables handling in NASM/
    */
    if (!h) return;

    fprintf(f, "section .bss\n");

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry *entry = &h->table[i];
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
            }
        }
    }

    fprintf(f, "\n");
}

void genParametreStockage(Node *node, FILE *f) {
    /*
    Generates function's parameters handling in NASM.
    */
    if (!node) return;

    int i = 0;
    for (Node *p = node->firstChild; p; p = p->nextSibling) {
        Node *idNode = NULL;
        for (Node *c = p->firstChild; c; c = c->nextSibling) {
            if (c->label == id) {
                idNode = c;
            }
        }

        if (idNode) {
            fprintf(f, "    mov [%s], %s\n", idNode->value.val_str, ArgumentsRegs2[i]);
        }
        i++;
    }
}

int isMainFunction(Node *node) {
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

void parcoursArbre(Node *n, FILE *f) {
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
            
            fprintf(f, "global _start\n\nsection .text\n");
            fprintf(f, "extern my_getchar\n"
                        "extern my_putchar\n"
                        "extern my_getint\n"
                        "extern my_putint\n");
            fprintf(f, "\n_start:\n");
            fprintf(f, "    call _main\n");

            fprintf(f, "    mov rdi, rax\n    mov rax, 60\n    syscall\n");
        // Functions
        case declFoncts:
            for (Node *declFonct = declFunctions->firstChild; declFonct; declFonct = declFonct->nextSibling) {
                Node *enTete = declFonct->firstChild;
                Node *corps = enTete->nextSibling;

                // Label fonction
                if (declFonct->symTable) fprintf(f, "_%s:\n", declFonct->symTable->functionName);
                Node *params = NULL;
                // Paramètres
                for (Node *c = enTete->firstChild; c; c = c->nextSibling) {
                    if (c->label == Parametres) {
                        params = c;
                    }
                }
                genParametreStockage(params, f);

                genInstr(corps->firstChild->nextSibling, f);
                fprintf(f, "    mov rax, 0\n    ret\n");
            }
        default:
            break;
    }
}