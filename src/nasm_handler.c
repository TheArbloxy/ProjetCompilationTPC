#include "nasm_handler.h"

void genExp(Node *node, FILE *f) {
    if (!node) return;

    switch (node->label) {
        // constante
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

        // cas des additions et soustractions
        case E: {
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
            fprintf(f, "    pop rsi\n"); // gauche

            if (op->label == add)
                fprintf(f, "    add rsi, rbx\n");
            else if (op->label == sub)
                fprintf(f, "    sub rsi, rbx\n");

            fprintf(f, "    push rsi\n");
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

void genInstr(Node *node, FILE *f) {
    if (!node) return;

    if (node->label == assign) {
        Node *expr = node->firstChild->nextSibling;

        genExp(expr, f);

        // résultat au sommet de la pile
        fprintf(f, "    pop rsi\n");
    }

    // parcourir récursivement
    for (Node *child = node->firstChild; child; child = child->nextSibling) {
        genInstr(child, f);
    }
}

int isMainFunction(Node *node) {
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

void parcoursArbre(Node *node, FILE *f) {
    if (!node) return;

    if (node->label == declFonct) {
        if (isMainFunction(node)) {
            fprintf(f, "global _start\nsection .text\n_start:\n");
            Node *corps = node->firstChild->nextSibling;
            genInstr(corps, f);

            fprintf(f, "    mov rax, 60\n    mov rdi, 0\n    syscall");
        }
    }

    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        parcoursArbre(child, f);
    }
}