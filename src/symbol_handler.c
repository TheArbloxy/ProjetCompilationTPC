#include "symbol_handler.h"
#include "symbol_table.h"
#include "tree.h"

static int globalAddress = 0;
static int semantic = 1; // Flag qui détecte si il n'y a pas d'erreur sémantique

static void handleDeclVars(Node *declVars, HashTable *table) {
    /*
    Handles variable declarations from the AST, to the symbol table.
    */
    for (Node *decl = declVars->firstChild; decl; decl = decl->nextSibling) {

        Node *typeNode = decl->firstChild;
        Node *ids = typeNode->nextSibling;

        for (Node *id = ids->firstChild; id; id = id->nextSibling) {
            Symbol s = {0};
            
            switch (typeNode->label) {
                case typeInt:
                    s.typ = SYM_INT;
                    s.Value.value_int = id->value.val_int;
                    break;
                case typeChar:
                    s.typ = SYM_CHAR;
                    s.Value.value_char = id->value.val_char;
                    break;
                default:
                    s.typ = SYM_NONE;
            }

            if (isGlobalScope(table)) {
                s.address = globalAddress;
                s.isGlobal = 1;
            } else {
                s.address = table->relativeAddress;
                s.isGlobal = 0;
            }

            if (!insert(table, id->value.val_str, s)) {
                printf("Erreur ligne %d : variable %s déjà déclarée\n",
                       id->lineno, id->value.val_str);
                semantic = 0;
            } else {
                if (isGlobalScope(table)) {
                    s.address += sizeofType(s.typ);
                } else {
                    table->relativeAddress += sizeofType(s.typ);
                }
            }
        }
    }
}


static void handleParams(Node *params, HashTable *table) {
    /*
    Handles function's parameters from the AST, to the symbol tree.
    */
    Node *list = params->firstChild;
    if (!list) return;

    for (Node *param = list->firstChild; param; param = param->nextSibling) {

        Node *typeNode = param->firstChild;
        Node *idNode   = typeNode->nextSibling;

        Symbol s = {0};

        if (typeNode->label == typeInt)
            s.typ = SYM_INT;
        else if (typeNode->label == typeChar)
            s.typ = SYM_CHAR;
        else
            s.typ = SYM_NONE;

        if (isGlobalScope(table)) {
            s.address = globalAddress;
            s.isGlobal = 1;
        } else {
            s.address = table->relativeAddress;
            s.isGlobal = 0;
        }

        if (!insert(table, idNode->value.val_str, s)) {
            printf("Erreur ligne %d : paramètre %s déjà déclaré\n",
                   idNode->lineno, idNode->value.val_str);
            semantic = 0;
        } else {
            if (isGlobalScope(table)) {
                s.address += sizeofType(s.typ);
            } else {
                table->relativeAddress += sizeofType(s.typ);
            }
        }
    }
}

static Symbol handleEval(Node *n, HashTable *table) {
    /*
    Handles evalutations from the AST, to the symbol tree.
    */
    if (!n) {
        return makeIntSymbol(0);
    }

    switch (n->label) {
        // Char constants
        case character:
            return makeCharSymbol(n->value.val_char);
        // Int constants
        case id: {
            return makeIntSymbol(n->value.val_int);
        }
        // Opérations binaires
        case Exp:
        case TB:
        case FB:
        case M:
        case E:
        case T: {

            Node *left = n->firstChild;
            Node *op = left->nextSibling;
            Node *right = op->nextSibling;

            Symbol s1 = handleEval(left, table);
            Symbol s2 = handleEval(right, table);

            Symbol result = {0};
            result.typ = SYM_INT;

            switch (op->label) {

                case add:
                    result.Value.value_int =
                        s1.Value.value_int +
                        s2.Value.value_int;
                    break;

                case sub:
                    result.Value.value_int =
                        s1.Value.value_int -
                        s2.Value.value_int;
                    break;

                case mul:
                    result.Value.value_int =
                        s1.Value.value_int *
                        s2.Value.value_int;
                    break;

                case divstar:
                    result.Value.value_int =
                        s1.Value.value_int /
                        s2.Value.value_int;
                    break;

                case mod:
                    result.Value.value_int =
                        s1.Value.value_int %
                        s2.Value.value_int;
                    break;

                case equals:
                    result.Value.value_int =
                        (s1.Value.value_int ==
                         s2.Value.value_int);
                    break;

                case notEquals:
                    result.Value.value_int =
                        (s1.Value.value_int !=
                         s2.Value.value_int);
                    break;

                case orderInf:
                    result.Value.value_int =
                        (s1.Value.value_int <
                         s2.Value.value_int);
                    break;

                case orderSup:
                    result.Value.value_int =
                        (s1.Value.value_int >
                         s2.Value.value_int);
                    break;

                case andExp:
                    result.Value.value_int =
                        (s1.Value.value_int &&
                         s2.Value.value_int);
                    break;

                case orExp:
                    result.Value.value_int =
                        (s1.Value.value_int ||
                         s2.Value.value_int);
                    break;

                default:
                    printf("Operateur inconnu\n");
                    break;
            }


            return result;
        }
        // Opérations unaires
        case unaryminus: {
            Symbol s = handleEval(n->firstChild, table);
            s.Value.value_int = -s.Value.value_int;
            return s;
        }
        case unaryplus:
            return handleEval(n->firstChild, table);
        case notInstr: {
            Symbol s = handleEval(n->firstChild, table);
            s.Value.value_int = !s.Value.value_int;
            return s;
        }
        // Accès à un champ
        case fieldAccess: {
            Node *idNode = n->firstChild;
            Symbol *s = lookup(table, idNode->value.val_str);
            if (!s) {
                printf("Erreur ligne %d : paramètre %s non déclaré\n",
                       idNode->lineno ,idNode->value.val_str);
                semantic = 0;

                return makeIntSymbol(0);
            }
            printf("SYMBOL : %d\n", s->Value.value_int ? s->Value.value_int : -999);
            return *s;
        }
        // Appel fonction
        case appelFonct: {
            Node *functionName = n->firstChild;
            Symbol *s = lookup(table, functionName->value.val_str);
            if (!s) {
                printf("Erreur ligne %d : fonction %s non déclaré\n",
                       functionName->lineno ,functionName->value.val_str);
                semantic = 0;

                return makeIntSymbol(0);
            }
            printf("FUNCTION\n");
            return *s;
        }

        default:
            printf("Label non géré : %s\n",
                   strToLabel(n->label));

            return makeIntSymbol(0);
    }
}

static void handleFunction(Node *n, HashTable *table) {
    /*
    Handles a function from the AST, to the symbol tree.
    */
    Symbol f = {0};
    Node *signature = n->firstChild; // Function's signature
    Node *corpse    = signature->nextSibling; // Function's corpse

    Node *functType = signature->firstChild; // Function type
    Node *functName = functType->nextSibling; // Function name

    // printf("LABEL TYPE : %s\n", functName ? functName->value.val_str : "null"); // TEST

    // Check first if the function is a refedinition
    if (lookup(table, functName->value.val_str)) {
        printf("Erreur ligne %d : fonction %s déjà définie\n",
            n->lineno, functName->value.val_str);
        semantic = 0;
        return;
    }

    // Creates local symbol table for the function
    n->symTable = calloc(1, sizeof(HashTable));
    initHashTable(n->symTable, table, functName->value.val_str, 4);

    // Check function type
    f.typ = SYM_FUNCTION;
    switch (functType->label) {
        case typeInt:
            f.Value.return_type = RETURN_INT;
            break;
        case typeChar:
            f.Value.return_type = RETURN_CHAR;
            break;
        default:
            f.Value.return_type = RETURN_VOID;
            break;
    }
    f.address = -1;
    f.isGlobal = 1;
    insert(table, functName->value.val_str, f);

    // Function parameters
    Node *params = functName->nextSibling;
    if (params) {
        handleParams(params, n->symTable);
    }

    // Local variables
    Node *declVars = corpse->firstChild;
    Node *suiteInstr = declVars->nextSibling;

    // printf("LABEL DECL : %s\n", strToLabel(declVars->label)); // TEST
    // printf("LABEL SUITE : %s\n", strToLabel(suiteInstr->label)); // TEST

    // Déclarations de variables dans la fonction
    if (declVars) {
        handleDeclVars(declVars, n->symTable);
    }
    // Suite des instructions dans la fonction (assign)
    if (suiteInstr) {
        for (Node *instr = suiteInstr->firstChild; instr; instr = instr->nextSibling) {
            switch (instr->label) {
                case assign: {
                    Node *lhs = instr->firstChild;
                    Node *rhs = lhs->nextSibling;
                    
                    // Récupérer la valeur de l'expression
                    Symbol value = handleEval(rhs, n->symTable);
                    Node *variableName = lhs->firstChild;

                    // Récupérer variable destination
                    Symbol *varDest = lookup(n->symTable, variableName->value.val_str);
                    if (!varDest) {
                        printf("Erreur ligne %d : paramètre %s non déclaré\n",
                            instr->lineno, variableName->value.val_str);
                        semantic = 0;
                        break;
                    }

                    // Vérification type
                    if (!castCheck(varDest->typ, value.typ)) {
                        printf("Avertissement ligne %d : conversion interdite de paramètre %s (int -> char)\n",
                            instr->lineno, variableName->value.val_str);
                        break;
                    }

                    Symbol finalValue = castSymbol(*varDest, value);
                    // Modifier la valeur dans la table
                    if (!lookupModify(n->symTable, variableName->value.val_str, finalValue)) {
                        printf("Erreur ligne %d : modification du paramètre %s échoué\n",
                            instr->lineno, variableName->value.val_str);
                        semantic = 0;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

int buildSymbolTables(Node *n, HashTable *table) {
    /*
    Builds all symbol tables, by linking them to the current node.
    It starts from the prog node, current handling global variables and functions.
    */
    if (!n) return 1;
    // printf("LABEL N : %s\n", n ? strToLabel(n->label) : "null");

    Node *declVars      = n->firstChild;
    Node *declFunctions = declVars->nextSibling;

    switch (n->label) {
        // Global variables
        case prog:
            n->symTable = table;
            handleDeclVars(declVars, table);
        // Functions
        case declFoncts:
            for (Node *declFonct = declFunctions->firstChild; declFonct; declFonct = declFonct->nextSibling) {
                // printf("LABEL FUNCT : %s\n", declFonct ? strToLabel(declFonct->label) : "null"); // TEST
                handleFunction(declFonct, table);
            }
        default:
            break;
    }
    return semantic;
}

void printAllTables(Node *n) {
    /*
    Prints all hash tables from nodes.
    */
    if (!n) return;

    if (n->symTable) {
        printf("==========================================================================\n");
        printf("Table - %s\n", n->symTable->functionName ? n->symTable->functionName : "null");
        printf("==========================================================================\n");
        printHashTable(n->symTable);
        printf("\n");
    }

    printAllTables(n->firstChild);
    printAllTables(n->nextSibling);
}