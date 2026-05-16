#include "helpers.h"

// HELPERS SYMBOL HANDLER //

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
    return !(LValue == SYM_CHAR && RValue == SYM_INT);
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