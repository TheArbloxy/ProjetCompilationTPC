#include "helpers.h"

// HELPERS SYMBOL HANDLER //

void printAllTables(Node *n) {
    /*
    Prints all hash tables from nodes.
    */
    if (!n) return;

    if (n->symTable) {
        printf("==========================================================================\n");
        printf("Table - %s\n", n->symTable->functionName ? n->symTable->functionName : "null");
        printf("==========================================================================\n");
        printf("Variables et fonctions :\n");
        printVariablesAndFunctions(n->symTable);
        printf("\n");  

        printf("Variables structures :\n");
        printStructureVariables(n->symTable);
        printf("\n");

        printf("Structures déclarées :\n");
        printStructScope(n->symTable->structs);
        printf("\n");  
    }

    printAllTables(n->firstChild);
    printAllTables(n->nextSibling);
}

extern Symbol makeIntSymbol(int v) {
    /*
    Handles a direct access to an int constant.
    */
    Symbol s = {0};
    s.typ = SYM_INT;
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
    s.address = -1;
    s.isGlobal = 0;
    return s;
}

extern int castCheck(TypeValue LValue, TypeValue RValue) {
    /*
    Checks if a cast is allowed (char -> int).
    However, (int -> char) isn't allowed by our compiler.
    */
    return !(LValue == SYM_CHAR && RValue == SYM_INT);
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

// HELPERS NASM HANDLER //

extern const char* getReserveDirective(TypeValue t) {
    switch (t) {
        case SYM_INT:
            return "resd"; // 4 bytes
        case SYM_CHAR:
            return "resb"; // 1 byte
        default:
            return "resq";
    }
}

extern int isBooleanExp(Node *node) {
    if (!node) return 0;

    switch (node->label) {
        case Exp:
        case TB:
        case FB:
        case M:
        case notInstr:
        case equals:
        case notEquals:
        case orderInf:
        case orderInfEquals:
        case orderSup:
        case orderSupEquals:
        case andExp:
        case orExp:
            return 1;
        
        default:
            return 0;
    }
}

extern int numberArgs(Node *node) {
    if (!node) return 0;
    
    int n = 0;
    for (Node *c = node->firstChild; c; c = c->nextSibling, n++);
    return n;
}

extern int getLocalStackTable(HashTable *h) {
    return -(h->relativeAddress);
}

extern int align16(int n) {
    return ((n + 15) / 16) * 16;
}

// Runtime functions

extern void write_runtime_bss(FILE *f) {
    fprintf(f, "    __charinput resb 1\n    __buffer resb 1\n    __input resb 1\n");
} 

extern void write_my_getchar(FILE *f) {
    fprintf(f, "\nmy_getchar: ; read (stdin, __charinput, 1)\n"
                "   mov rax, 0\n"
                "   mov rdi, 0\n"
                "   mov rsi, __charinput\n"
                "   mov rdx, 1\n"
                "   syscall\n\n"

                "    ; renvoyer le caractère lu\n"
                "   movzx rax, byte [__charinput]\n"
                "   ret\n\n"
    );
} 

extern void write_my_putchar(FILE *f) {
    fprintf(f, "my_putchar: ; stocke le caractère\n"
                "   mov [__buffer], dil\n\n"

                "   ; write (stdout, __buffer, 1)\n"
                "   mov rax, 1\n"
                "   mov rdi, 1\n"
                "   mov rsi, __buffer\n"
                "   mov rdx, 1\n"
                "   syscall\n\n"

                "   ret\n\n"
    );
}

extern void write_my_getint(FILE *f) {
    fprintf(f, "\nmy_getint:\n"
                "   push rbx\n"
                "   xor rbx, rdx ; résultat = 0\n\n"

                ".read_first:\n"
                "   ; read (stdin, __input, 1)\n"
                "   mov rax, 0\n"
                "   mov rdi, 0\n"
                "   mov rsi, __input\n"
                "   mov rdx, 1\n"
                "   syscall\n\n"

                "   mov al, [__input]\n\n"

                "   ; vérifier chiffre\n"
                "   cmp al, '0'\n"
                "   jl .error\n\n"

                "   cmp al, '9'\n"
                "   jg .error\n"

                ".loop:\n"
                "   ; result = result * 10\n"
                "   imul rbx, rbx, 10\n"

                "   ; convertir ASCII -> entier\n"
                "   movzx rax, al\n"
                "   sub rax, '0'\n"
                "   add rbx, rax\n\n"

                "   ; lire caractère suivant\n"
                "   mov rax, 0\n"
                "   mov rdi, 0\n"
                "   mov rsi, __input\n"
                "   mov rdx, 1\n"
                "   syscall\n\n"

                "   mov al, [__input]\n"

                "   ; continuer si chiffre\n\n"
                "   cmp al, '0'\n"
                "   jl .done\n\n"

                "   cmp al, '9'\n"
                "   jl .done\n\n"

                "   jmp .loop\n"

                ".done:\n"
                "   mov rax, rbx\n"
                "   pop rbx\n"
                "   ret\n"

                ".error:\n"
                "   mov rax, 60\n"
                "   mov rdi, 5\n"
                "   syscall\n"
        );
}

extern void write_my_putint(FILE *f) {
    fprintf(f,  "\nmy_putint:\n"

                "   push rbx\n"
                "   push r12\n\n"

                "   mov rax, rdi\n"
                "   xor r12, r12\n\n"

                "   ; négatif\n"
                "   cmp rax, 0\n"
                "   jge .check_zero\n"

                "   neg rax\n"

                "   mov dil, '-'\n"
                "   call my_putchar\n"

                ".check_zero:\n"

                "   cmp rax, 0\n"
                "   jne .extract\n"

                "   mov dil, '0'\n"
                "   call my_putchar\n"
                "   jmp .done\n"

                ".extract:\n"

                "   mov rbx, 10\n"

                ".loop:\n"

                "   xor rdx, rdx\n"
                "   div rbx\n"

                "   push rdx\n"
                "   inc r12\n"

                "   cmp rax, 0\n"
                "   jne .loop\n"

                ".print:\n"

                "   pop rdx\n"

                "   add dl, '0'\n"

                "   mov dil, dl\n"
                "   call my_putchar\n"

                "   dec r12\n"
                "   jnz .print\n"

                ".done:\n"

                "   pop r12\n"
                "   pop rbx\n"

                "   ret\n"
    );
}