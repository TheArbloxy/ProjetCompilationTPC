#include <stdio.h>
#include <stdlib.h>
#include "symbol_handler.h"

static int globalAddress = 0;
static int semanticErrorCount = 0;

static void handleDeclStruct(Node *n, HashTable *table) {
    /*
    Handles structure declarations from the AST, to the symbol table.
    */
    if (!n || !n->firstChild) return;

    Node *ident = n->firstChild->nextSibling;
    if (!ident) return;
    Node *declStr = ident->nextSibling;
    if (!declStr) return;

    switch (declStr->label) {
        case declStruct: {
            StructDef def = {0};
            def.structName = strdup(ident->value.val_str);

            int currentOffset = 0;

            // Parcourir les déclarations de structure
            for (Node* fieldDecl = declStr->firstChild; fieldDecl; fieldDecl = fieldDecl->nextSibling) {
                Node *fieldType = fieldDecl->firstChild;
                Node *fieldIds = fieldType->nextSibling;

                // Parcourir les champs de la structure
                for (Node* field = fieldIds->firstChild; field; field = field->nextSibling) {
                    StructEntry entry = {0};
                    if (field->value.val_str) {
                        entry.key = strdup(field->value.val_str);
                    } else {
                        entry.key = strdup("test");
                    }
                    entry.offset = currentOffset;

                    // Type champ
                    switch(fieldType->label) {
                        case typeInt:
                            entry.symbol.typ = SYM_INT;
                            entry.size = 4;
                            break;
                        case typeChar:
                            entry.symbol.typ = SYM_CHAR;
                            entry.size = 1;
                            break;
                        case typeStruct: { // TODO : fix
                            entry.symbol.typ = SYM_STRUCT;
                            StructDef *nested = lookupStruct(table, fieldType->nextSibling->value.val_str);
                            if (!nested) {
                                printf("Erreur ligne %d : structure %s non déclarée\n",
                                    ident->lineno, ident->value.val_str);
                                    semanticErrorCount++;
                                continue; 
                            }

                            entry.symbol.structName = strdup(nested->structName);
                            entry.size = nested->totalSize;
                            break;
                        }
                        default:
                            break;
                    }
                    currentOffset += entry.size;
                    printf("OFFSET : %d\n", currentOffset);
                    insertField(&def, entry);
                }
            } 

            def.totalSize += currentOffset;
            insertStruct(table, def);
            break;
        }
        case declarateurs: {
            StructDef *def = lookupStruct(table, ident->value.val_str);
            if (!def) {
                printf("Erreur ligne %d : structure %s non déclarée\n",
                ident->lineno, ident->value.val_str);
                semanticErrorCount++;
                break;
            }

            // Déclarateurs variables
            for (Node *id = declStr->firstChild; id; id = id->nextSibling) {
                SymbolS s = {0};
                s.structName = strdup(def->structName);
                s.size = def->totalSize;

                printf("TYPE STRUCT = %s | SIZE = %d\n", s.structName, s.size);
                printf("VAR = %s\n", id->value.val_str);

                // Check scope
                if (isGlobalScope(table)) {
                    s.address = globalAddress;
                    s.isGlobal = 1;
                } else {
                    s.address = table->relativeAddress;
                    s.isGlobal = 0;
                }

                if (!insertStructVariable(table, id->value.val_str, s)) {
                    printf("Erreur ligne %d : variable %s déjà déclarée\n",
                        id->lineno, id->value.val_str);
                    semanticErrorCount++;
                } else {
                    if (isGlobalScope(table)) {
                        s.address += def->totalSize;
                    } else {
                        table->relativeAddress += def->totalSize;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

static void handleDeclVars(Node *declVars, HashTable *table) {
    /*
    Handles variable declarations from the AST, to the symbol table.
    */
    for (Node *decl = declVars->firstChild; decl; decl = decl->nextSibling) {
        switch (decl->label) {
            // Déclaration de variable
            case declVar: {
                Node *typeNode = decl->firstChild;
                Node *ids = typeNode->nextSibling;

                for (Node *id = ids->firstChild; id; id = id->nextSibling) {
                    Symbol s = {0};
                    
                    // Check type
                    switch (typeNode->label) {
                        case typeInt:
                            s.typ = SYM_INT;
                            break;
                        case typeChar:
                            s.typ = SYM_CHAR;
                            break;
                        default:
                            s.typ = SYM_NONE;
                    }

                    // Check scope
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
                        semanticErrorCount++;
                    } else {
                        if (isGlobalScope(table)) {
                            s.address += sizeofType(s.typ);
                        } else {
                            table->relativeAddress += sizeofType(s.typ);
                        }
                    }
                }
                break;
            // Déclaration structure
            } case declVarStruct: {
                handleDeclStruct(decl, table);
                break;
            } default: 
                break;
                
        }
    }
}


static void handleParams(Node *params, HashTable *table, FunctionInfo *f) {
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
            semanticErrorCount++;
        } else {
            if (isGlobalScope(table)) {
                s.address += sizeofType(s.typ);
            } else {
                table->relativeAddress += sizeofType(s.typ);
            }
            // Récupérer type
            f->paramTypes[f->numberParams] = s.typ;
            if (s.typ != SYM_NONE) f->numberParams++;
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

            handleEval(left, table);
            handleEval(right, table);

            Symbol result = {0};
            result.typ = SYM_INT;
            
            return result;
        }
        // Opérations unaires
        case unaryminus:
        case unaryplus:
        case notInstr:
            return handleEval(n->firstChild, table);
        // Accès à un champ
        case fieldAccess: {
            Node *idNode = n->firstChild;
            Symbol *s = lookup(table, idNode->value.val_str);
            if (!s) {
                printf("Erreur ligne %d : variable %s non déclarée\n",
                       idNode->lineno ,idNode->value.val_str);
                semanticErrorCount++;

                return makeIntSymbol(0);
            }
            return *s;
        }
        // Appel fonction
        case appelFonct: {
            Node *functionName = n->firstChild;
            Node *arguments = functionName->nextSibling;

            // Non de fonction
            Symbol *s = lookupFunction(table, functionName->value.val_str);
            if (!s) {
                printf("Erreur ligne %d : fonction %s non déclarée\n",
                       functionName->lineno ,functionName->value.val_str);
                semanticErrorCount++;

                return makeIntSymbol(0);
            }

            // Check arguments
            if (s->value_funct.numberParams > numberArgs(arguments)) {
                printf("Erreur ligne %d : pas assez d'arguments à la fonction %s, %d expecté, %d reçu\n",
                       functionName->lineno ,functionName->value.val_str,
                        s->value_funct.numberParams, numberArgs(arguments));
                semanticErrorCount++;

                return makeIntSymbol(0);

            } else if (s->value_funct.numberParams < numberArgs(arguments)) {
                printf("Erreur ligne %d : trop d'arguments à la fonction %s, %d expecté, %d reçu\n",
                       functionName->lineno, functionName->value.val_str,
                        s->value_funct.numberParams, numberArgs(arguments));
                semanticErrorCount++;

                return makeIntSymbol(0);
            }
            
            // Check arguments
            int i = 0;
            for (Node *arg = arguments->firstChild; arg; arg = arg->nextSibling, i++) {
                Symbol a = handleEval(arguments->firstChild, table);
                if (!castCheck(s->value_funct.paramTypes[i], a.typ)) {
                    printf("Erreur ligne %d : conversion interdite d'argument de la fonction %s\n",
                        functionName->lineno, functionName->value.val_str);
                    semanticErrorCount++;
                }

                return makeIntSymbol(0);
            }

            return *s;
        }
        default:
            printf("Label non géré : %s\n",
                   strToLabel(n->label));

            return makeIntSymbol(0);
    }
}

static void handleReturnType(Node *n, Node *functionName, Symbol function, HashTable *table) {
    /*
    Handles a return case from the AST, to the symbol tree.
    */
    Symbol ident = handleEval(n->firstChild, table);
    TypeValue returnType = function.value_funct.returnType;

    if (!castCheck(returnType, ident.typ)) {
        printf("Avertissement ligne %d : conversion interdite de valeur de retour de la fonction %s\n",
            n->lineno, functionName->value.val_str);
    }
}

static void handleAssign(Node *n, Node *instr, HashTable *table) {
    /*
    Handles an assign case from the AST, to the symbol tree.
    */
    Node *lhs = instr->firstChild;
    Node *rhs = lhs->nextSibling;
    
    // Récupérer la valeur de l'expression
    Symbol value = handleEval(rhs, n->symTable);
    Node *variableName = lhs->firstChild;

    // Récupérer variable destination
    Symbol *varDest = lookup(n->symTable, variableName->value.val_str);
    if (!varDest) {
        printf("Erreur ligne %d : variable %s non déclarée\n",
            instr->lineno, variableName->value.val_str);
        semanticErrorCount++;
        return;
    }

    // Vérification type
    if (!castCheck(varDest->typ, value.typ)) {
        printf("Avertissement ligne %d : conversion interdite de variable %s (int -> char)\n",
            instr->lineno, variableName->value.val_str);
        return;
    }
    if ((value.typ == SYM_FUNCTION || value.typ == SYM_BUILTIN) && value.value_funct.returnType == SYM_NONE) {
        printf("Erreur ligne %d : variable %s assigné à un type incompatible 'void'\n",
            instr->lineno, variableName->value.val_str);
        semanticErrorCount++;
        return;
    }

    castSymbol(*varDest, value);
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

    int hasReturn = 0;

    // printf("LABEL TYPE : %s\n", functName ? functName->value.val_str : "null"); // TEST

    // Check first if the function is a refedinition
    if (lookupFunction(table, functName->value.val_str)) {
        printf("Erreur ligne %d : fonction %s déjà définie\n",
            n->lineno, functName->value.val_str);
        semanticErrorCount++;
        return;
    }
    if (lookup(table, functName->value.val_str)) {
        printf("Erreur ligne %d : redéfinition de l'espace de nommage %s\n",
            n->lineno, functName->value.val_str);
        semanticErrorCount++;
        return;
    }

    // Creates local symbol table for the function
    n->symTable = calloc(1, sizeof(HashTable));
    initHashTable(n->symTable, table, functName->value.val_str, 4);

    // Check function type
    f.typ = SYM_FUNCTION;
    switch (functType->label) {
        case typeInt:
            f.value_funct.returnType = SYM_INT;
            break;
        case typeChar:
            f.value_funct.returnType = SYM_CHAR;
            break;
        default:
            f.value_funct.returnType = SYM_NONE;
            break;
    }

    f.address = -1;
    f.isGlobal = 1;

    // Function parameters
    Node *params = functName->nextSibling;
    if (params) {
        handleParams(params, n->symTable, &f.value_funct);
    }

    insert(table, functName->value.val_str, f);

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
                    handleAssign(n, instr, n->symTable);
                    break;
                }
                case appelFonct: {
                    handleEval(instr, n->symTable);
                    break;
                }
                case voidSt: {
                    // Check si c'est une fonction void
                    if (f.value_funct.returnType != SYM_NONE) {
                        printf("Erreur ligne %d : La fonction non-void %s devrait renvoyer une valeur\n",
                            instr->lineno, functName->value.val_str);
                        semanticErrorCount++;
                    }
                    break;
                }
                case returnSt: {
                     // Check si c'est une fonction void
                    if (f.value_funct.returnType == SYM_NONE) {
                        printf("Erreur ligne %d : La fonction void %s ne devrait pas renvoyer une valeur\n",
                            instr->lineno, functName->value.val_str);
                        semanticErrorCount++;
                    } else {
                        handleReturnType(instr, functName, f, n->symTable);
                        hasReturn = 1;
                    }
                    break;
                }
                default:
                    printf("Test: %s\n", instr ? strToLabel(instr->label) : "null"); // TEST
                    break;
            }
        }
    }

    if (f.value_funct.returnType != SYM_NONE) {
        if (!hasReturn) {
            printf("Avertissement ligne %d : La fonction non-void %s ne renvoie pas de valeur\n",
                n->lineno, functName->value.val_str);
        }
    }
}

static void checkMain(HashTable *table) {
    /*
    Checks if the 'main' function exists, and returns an int.
    */
    Symbol *main = lookupFunction(table, "main");
    if (!main){
        printf("Erreur : pas de fonction main détectée\n");
        semanticErrorCount++;
        return;
    }
    if (main && main->value_funct.returnType != SYM_INT) {
        printf("Erreur : la fonction main doit renvoyer un int\n");
        semanticErrorCount++;
    }
}

int buildSymbolTables(Node *n, HashTable *table){
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
                handleFunction(declFonct, table);
            }
        default:
            break;
    }

    // printAllTables(n);
    checkMain(table);

    printf("Erreurs sémantiques : %d\n", semanticErrorCount);
    return semanticErrorCount ? 0 : 1; // Check s'il y a au moins une erreur sémantique
}