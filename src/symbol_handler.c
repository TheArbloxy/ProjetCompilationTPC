#include "symbol_handler.h"
#include "symbol_table.h"
#include "tree.h"

static void handleDeclVars(Node *declVars, HashTable *table) {
    /*
    Handles variable declarations from the AST, to the symbol table.
    */
    for (Node *decl = declVars->firstChild; decl; decl = decl->nextSibling) {

        Node *typeNode = decl->firstChild;
        Node *ids = typeNode->nextSibling;

        for (Node *id = ids->firstChild; id; id = id->nextSibling) {
            Symbol s;
            
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
            
            if (!insert(table, id->value.val_str, s)) {
                printf("Erreur ligne %d : variable %s déjà déclarée\n",
                       id->lineno, id->value.val_str);
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

        Symbol s;

        if (typeNode->label == typeInt)
            s.typ = SYM_INT;
        else if (typeNode->label == typeChar)
            s.typ = SYM_CHAR;
        else
            s.typ = SYM_NONE;

        if (!insert(table, idNode->value.val_str, s)) {
            printf("Erreur ligne %d : paramètre %s déjà déclaré\n",
                   idNode->lineno, idNode->value.val_str);
        }
    }
}

static Symbol* handleFieldAccess(Node *n, HashTable *table) {
    /*
    Handles a field access from the AST.
    */
    printf("LOOKUP FOR : %s\n", n->value.val_str);
    Symbol *s = lookup(table, n->value.val_str);
    if (!s) {
        printf("Erreur ligne %d : paramètre %s non déclaré\n",
            n->lineno, n->value.val_str);
        return NULL;
    }
    return s;
}

static Symbol* handleBinaryOperations(Node *n, HashTable *table) {
    /*
    Handles all binary operations (+, -, *, /, %)
    */
    Node *firstOperande = n->firstChild;
    Node *operator = firstOperande->nextSibling;
    Node *secondOperande = operator->nextSibling;

    Symbol *s1 = NULL;
    Symbol *s2 = NULL;
    Symbol *result = NULL;

    // Récupérer le premier & second opérande
    switch (firstOperande->label) {
        case fieldAccess:
            s1 = handleFieldAccess(firstOperande, table);
            break;
        /*
        case id:
            // Accès direct TODO : fix seg fault
            switch (firstOperande->typ) {
                case VALUE_INT:
                    s1->typ = SYM_INT;
                    s1->Value.value_int = firstOperande->value.val_int;
                    break;
                case VALUE_CHAR:
                    s1->typ = SYM_CHAR;
                    s1->Value.value_int = firstOperande->value.val_char;
                    break;
                default:
                    break;
            }
            break;
        */
        default:
            break;
    }

    switch (secondOperande->label) {
        case fieldAccess:
            s2 = handleFieldAccess(secondOperande, table);
            break;
        /*
        case id:
            // Accès direct TODO : fix seg fault
            switch (firstOperande->typ) {
                case VALUE_INT:
                    s2->typ = SYM_INT;
                    s2->Value.value_int = firstOperande->value.val_int;
                    break;
                case VALUE_CHAR:
                    s2->typ = SYM_CHAR;
                    s2->Value.value_int = firstOperande->value.val_char;
                    break;
                default:
                    break;
            }
            break;
        */
        default:
            break;
    }

    // Appliquer l'opérateur
    if (s1 && s2)  {
        result->typ = SYM_INT; // TEMPORARY
        switch (operator->label) {
            case add:
                result->Value.value_int = s1->Value.value_int + s2->Value.value_int;
                break;
            case sub:
                result->Value.value_int = s1->Value.value_int - s2->Value.value_int;
                break;
            case mul:
                result->Value.value_int = s1->Value.value_int * s2->Value.value_int;
                break;
            case divstar:
                result->Value.value_int = s1->Value.value_int / s2->Value.value_int;
                break;
            case mod:
                result->Value.value_int = s1->Value.value_int % s2->Value.value_int;
                break;
            default:
                break;
        }
    }

    
    printf("FIRST OPERANDE : %s\n", firstOperande ? strToLabel(firstOperande->label) : "null"); // TEST
    printf("OPERATOR : %s\n", operator ? strToLabel(operator->label) : "null"); // TEST
    printf("SECOND OPERANDE : %s\n", secondOperande ? strToLabel(secondOperande->label) : "null"); // TEST
    

    return result;
}

static void handleAssign(Node *n, HashTable *table) {
    /*
    Handles an assign instruction from the AST, to the symbol tree.
    */
    Node *access = n->firstChild; // Access
    Node *accessIdent = access->firstChild; // LValue
    Node *ident = access->nextSibling; // RValue

    Symbol *s;
    Symbol *sExp;

    // printf("ACCESS TYPE : %s\n", access ? strToLabel(access->label) : "null"); // TEST
    // printf("IDENT TYPE : %s\n", ident ? strToLabel(ident->label) : "null"); // TEST

    switch (access->label) {
        case fieldAccess:
            // Vérifier que la variable de base existe (Gestion LValue)
            s = handleFieldAccess(accessIdent, table);

            // Vérifier l'expression (Gestion RValue)
            if (!ident) return;
            printf("ACCESS TYPE : %s\n", ident ? strToLabel(ident->label) : "null"); // TEST

            Node *accessExp;
            int fieldExpMod = 0; // Flag qui vérifie si l'expression est une variable qui existe

            switch (ident->label) {
                case fieldAccess: // Si l'expression est une variable qui existe déjà
                    accessExp = ident->firstChild;
                    sExp = handleFieldAccess(accessExp, table);
                    if (sExp) fieldExpMod = 1;
                    break;
                case E: // Si l'expression est une addition ou soustraction
                case T: // Si l'expression est une multiplication, divison ou modulo
                    sExp = handleBinaryOperations(ident, table);
                    if (sExp) fieldExpMod = 1;
                    break;
                default:
                    break;
            }
            
            if (fieldExpMod) {
                switch(sExp->typ) {
                    case VALUE_INT:
                        ident->typ = VALUE_INT;
                        ident->value.val_int = sExp->Value.value_int;
                        break;
                    case VALUE_CHAR:
                        ident->typ = VALUE_CHAR;
                        ident->value.val_char = sExp->Value.value_char;
                        break;
                    default:
                        break;
                }
            }
            
            // Affectation selon le type de la variable
            switch (ident->typ) {
                case VALUE_INT:
                    printf("MODIFY INT : %d\n", ident->value.val_int);
                    if (s->typ == SYM_CHAR) { // Si on modifie un char avec un int, on provoque un avertissement, et reste non initialisée
                        printf("Avertissement ligne %d : tentative de conversion du paramètre %s avec un int\n",
                            accessIdent->lineno, accessIdent->value.val_str);
                    } else {
                        s->Value.value_int = ident->value.val_int;
                    }
                    break;
                case VALUE_CHAR:
                    printf("MODIFY CHAR : %c\n", ident->value.val_char);
                    if (s->typ == SYM_INT) { // Si on modifie un int avec un char, on effectue la conversion
                        s->Value.value_int = ident->value.val_char;
                    } else {
                        s->Value.value_char = ident->value.val_char;
                    }
                    break;
                default:
                    break;

            }
            
            // Modifier la valeur dans la table
            if (!lookupModify(table, accessIdent->value.val_str, s)) {
                printf("Erreur ligne %d : paramètre %s non déclaré\n",
                    accessIdent->lineno, accessIdent->value.val_str);
            }

        default:
            break;
    }
}

static void handleFunction(Node *n, HashTable *table) {
    /*
    Handles a function from the AST, to the symbol tree.
    */
    // Creates local symbol table for the function
    n->symTable = malloc(sizeof(HashTable));
    initHashTable(n->symTable, table);

    Symbol f;
    Node *signature = n->firstChild; // Function's signature
    Node *corpse    = signature->nextSibling; // Function's corpse

    Node *functType = signature->firstChild; // Function type
    Node *functName = functType->nextSibling; // Function name

    // printf("LABEL TYPE : %s\n", functType ? strToLabel(functType->label) : "null"); // TEST

    // Check function type
    switch (functType->label) {
        case typeInt:
            f.typ = SYM_INT;
            f.Value.value_int = -67; // placeholder value
            break;
        case typeChar:
            f.typ = SYM_CHAR;
            f.Value.value_char = 'b'; // placeholder value
            break;
        default:
            f.typ = SYM_NONE; // VOID return
            break;

    }
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
            if (instr->label == assign) {
                handleAssign(instr, n->symTable);
            }
        }
    }
}

void buildSymbolTables(Node *n, HashTable *table) {
    /*
    Builds all symbol tables, by linking them to the current node.
    It starts from the prog node, current handling global variables and functions.
    */
    if (!n) return;
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
}

void printAllTables(Node *n) {
    /*
    Prints all hash tables from nodes.
    */
    if (!n) return;

    if (n->symTable) {
        printf("Table au noeud %s (%d):\n", strToLabel(n->label), n->lineno);
        printHashTable(n->symTable);
        printf("\n");
    }

    printAllTables(n->firstChild);
    printAllTables(n->nextSibling);
}