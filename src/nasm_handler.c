#include "nasm_handler.h"

static int labelCounter = 0;

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

            if (!left || !op || !right) {
                printf("Erreur AST E mal formé\n");
                return;
            }

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
            // Node *args = functionName->nextSibling;

            if (!functionName) return;

            // Fonctions builtin (getchar et getint)
            if (strcmp(functionName->value.val_str, "getchar") == 0) {
                fprintf(f, "    call my_getchar\n");
                fprintf(f, "    push rax\n");
            } else if (strcmp(functionName->value.val_str, "getint") == 0) {
                fprintf(f, "    call my_getint\n");
                fprintf(f, "    push rax\n");
            }
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

void genAssign(Node *node, FILE *f) {
    /*
    Generates assigns in NASM.
    */
    if (!node) return;

    Node *var = node->firstChild;
    Node *expr = var->nextSibling;

    // Gérer expression
    genExp(expr, f);
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
    }
}

void genInstr(Node *node, FILE *f) {
    /*
    Generates instructions in NASM.
    */
    if (!node) return;

    for (Node *child = node->firstChild; child; child = child->nextSibling) {
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
                Node *suiteInstr = cond->nextSibling;

                int labelEnd = labelCounter;
                labelCounter++;

                // Test de comparaison
                genExp(cond, f);
                fprintf(f, "    pop rax\n");

                fprintf(f, "    cmp rax, 0\n");
                fprintf(f, "    je .L%d\n", labelEnd);
                
                // Instructions si le if passe
                genInstr(suiteInstr, f);
                
                // Sinon, on fait un jump après
                fprintf(f, ".L%d:\n", labelEnd);
                break;
            }
            case elseSt: {
                Node *cond = child->firstChild;
                Node *thenInstr = cond->nextSibling;
                Node *elseInstr = thenInstr->nextSibling;

                int labelEnd = labelCounter;
                labelCounter++;

                // Test de comparaison
                genExp(cond, f);
                fprintf(f, "    pop rax\n");

                fprintf(f, "    cmp rax, 0\n");
                fprintf(f, "    je .Lelse_%d\n", labelEnd);

                // Instructions si le if passe, on jump le else
                genInstr(thenInstr, f);
                fprintf(f, "    jmp .Lendif_%d\n", labelEnd);

                // Instructions du else
                fprintf(f, ".Lelse_%d:\n", labelEnd);
                genInstr(elseInstr, f);

                // Fin
                fprintf(f, ".Lendif_%d:\n", labelEnd);
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
        // Functions
        case declFoncts:
            for (Node *declFonct = declFunctions->firstChild; declFonct; declFonct = declFonct->nextSibling) {
                if (isMainFunction(declFonct)) {
                    fprintf(f, "global _start\n\nsection .text\n");
                    fprintf(f, "extern my_getchar\n"
                                "extern my_putchar\n"
                                "extern my_getint\n"
                                "extern my_putint\n");
                    fprintf(f, "\n_start:\n");
                    Node *corps = declFonct->firstChild->nextSibling;
                    genInstr(corps->firstChild->nextSibling, f);

                    fprintf(f, "    mov rax, 60\n    mov rdi, 0\n    syscall");
                }
            }
        default:
            break;
    }
}