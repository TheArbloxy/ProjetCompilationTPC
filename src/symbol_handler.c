#include <stdio.h>
#include <stdlib.h>
#include "symbol_handler.h"

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
            // Check if the structure already exists in already declared structures from the current scope
            StructDef *checkStruct = lookupStruct(table, ident->value.val_str);
            // If yes, it's a semantic error
            if (checkStruct) {
                printf("Error line %d : structure %s already declared\n",
                ident->lineno, ident->value.val_str);
                semanticErrorCount++;
                break;
            }

            // Otherwise, we create the structure
            StructDef def = {0};
            def.structName = strdup(ident->value.val_str);
            if (isGlobalScope(table)) {
                def.isGlobal = 1;
            }

            int currentOffset = 0;

            // Iterates all structures declaration
            for (Node* fieldDecl = declStr->firstChild; fieldDecl; fieldDecl = fieldDecl->nextSibling) {
                Node *fieldType = fieldDecl->firstChild;

                Node *fieldIds = NULL;
                if (fieldType->label == typeStruct) {
                    fieldIds = fieldType->nextSibling->nextSibling;
                } else {
                    fieldIds = fieldType->nextSibling;
                }

                // Iterates all fields of the structure 
                for (Node* field = fieldIds->firstChild; field; field = field->nextSibling) {
                    StructEntry entry = {0};
                    if (field->value.val_str) {
                        entry.key = strdup(field->value.val_str);
                    } else {
                        entry.key = strdup("test");
                    }
                    entry.offset = currentOffset;

                    // Field type
                    switch(fieldType->label) {
                        case typeInt:
                            entry.typ = SYM_INT;
                            entry.size = 4;
                            break;
                        case typeChar:
                            entry.typ = SYM_CHAR;
                            entry.size = 1;
                            break;
                        case typeStruct: {
                            entry.typ = SYM_STRUCT;
                            // Nested structure
                            StructDef *nested = lookupStruct(table, fieldType->nextSibling->value.val_str);
                            if (!nested) {
                                printf("Error line %d : structure %s undeclared\n",
                                    ident->lineno, ident->value.val_str);
                                    semanticErrorCount++;
                                break; 
                            }

                            entry.structName = strdup(nested->structName);
                            entry.size = nested->totalSize;
                            break;
                        }
                        default:
                            break;
                    }
                    currentOffset += entry.size;
                    entry.offset = currentOffset;
                    insertField(&def, entry); // Insert field in the structure
                }
            } 

            def.totalSize += currentOffset;
            insertStruct(table, def); // Insert structure in structures declaration in the scope
            break;
        }
        case declarateurs: {
            // Check if the structure already exists in already declared structures from the current scope
            StructDef *def = lookupStruct(table, ident->value.val_str);
            // If not, it's a semantic error
            if (!def) {
                printf("Error line %d : structure %s undeclared\n",
                ident->lineno, ident->value.val_str);
                semanticErrorCount++;
                break;
            }

            // Variable declarators
            for (Node *id = declStr->firstChild; id; id = id->nextSibling) {
                SymbolS s = {0};
                s.structName = strdup(def->structName);
                s.size = def->totalSize;

                if (isGlobalScope(table)) {
                    s.isGlobal = 1;
                }

                // Check scope
                s.address = table->relativeAddress;

                if (!insertStructVariable(table, id->value.val_str, s)) {
                    printf("Error line %d : variable %s already declared\n",
                        id->lineno, id->value.val_str);
                    semanticErrorCount++;
                } else {
                    table->relativeAddress += def->totalSize;
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
            // Variables declaration
            case declVar: {
                Node *typeNode = decl->firstChild;
                Node *ids = typeNode->nextSibling;

                for (Node *id = ids->firstChild; id; id = id->nextSibling) {
                    Symbol s = {0};
                    if (isGlobalScope(table)) {
                        s.isGlobal = 1;
                    }
                    
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
                            break;
                    }

                    // Check scope
                    table->relativeAddress += sizeofType(s.typ);
                    s.address = table->relativeAddress;

                    // If insert failed, it's a semantic error
                    if (!insert(table, id->value.val_str, s)) {
                        printf("Error line %d : variable %s already declared\n",
                            id->lineno, id->value.val_str);
                        semanticErrorCount++;
                        table->relativeAddress -= sizeofType(s.typ);
                    }
                }
                break;
            // Structure variable declaration
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

    // Iterates though all parameters
    for (Node *param = list->firstChild; param; param = param->nextSibling) {
        Node *typeNode = param->firstChild;
        Node *idNode   = typeNode->nextSibling;

        Symbol s = {0};

        if (isGlobalScope(table)) {
            s.isGlobal = 1;
        }

        // Check type
        if (typeNode->label == typeInt)
            s.typ = SYM_INT;
        else if (typeNode->label == typeChar)
            s.typ = SYM_CHAR;
        else
            s.typ = SYM_NONE;

        table->relativeAddress += sizeofType(s.typ);
        s.address = table->relativeAddress;

        // If insert failed, it's a semantic error
        if (!insert(table, idNode->value.val_str, s)) {
            printf("Error line %d : parameter %s already declared\n",
                   idNode->lineno, idNode->value.val_str);
            semanticErrorCount++;
            table->relativeAddress -= sizeofType(s.typ);
        } else {
            // Get parameter's type
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
        // Binary operations
        case Exp:
        case TB:
        case FB:
        case M:
        case E:
        case T: {
            Node *left = n->firstChild;
            Node *op = left->nextSibling;
            Node *right = op->nextSibling;

            handleEval(left, table); // LValue
            handleEval(right, table); // RValue

            Symbol result = {0};
            result.typ = SYM_INT;
            
            return result;
        }
        // Unary operations
        case unaryminus:
        case unaryplus:
        case notInstr:
            return handleEval(n->firstChild, table);
        // Field access
        case fieldAccess: {
            if (numberArgs(n) > 1) { // Structure field (ig : p.a = 10;)
                Node *baseVar = n->firstChild;

                // Lookup for structure variable
                HashSEntry *baseStruct = lookupStructureVariable(table, baseVar->value.val_str);
                if (!baseStruct) {
                    printf("Error line %d : structure %s undeclared\n",
                        n->lineno, baseVar->value.val_str);
                    semanticErrorCount++;
                    return makeIntSymbol(0);
                }

                // Nested structure
                StructDef *currentStruct = lookupField(table, baseStruct->symbol.structName);
                if (!currentStruct) {
                    printf("Internal error : structure %s not found\n",
                        baseStruct->symbol.structName);
                    return makeIntSymbol(0);
                }
                
                // Next fields (ex : p.a; ou p.color.r;)
                Node *field = baseVar->nextSibling;
                StructEntry *fieldSymbol = NULL;

                while (field) {
                    fieldSymbol = lookupEntry(currentStruct, field->value.val_str);
                    if (!fieldSymbol) {
                        printf("Error line %d : field %s not found\n",
                            n->lineno, field->value.val_str);
                        semanticErrorCount++;
                        return makeIntSymbol(0);
                    }

                    // If nested field (ex : p.color.r) : must be a structure
                    if (field->nextSibling) {
                        if (!lookupStruct(table, fieldSymbol->structName)) {
                            printf("Error line %d : nested structure %s not found\n",
                                n->lineno, field->value.val_str);
                            semanticErrorCount++;
                            return makeIntSymbol(0);
                        }
                        currentStruct = lookupField(table, fieldSymbol->structName);
                    }

                    field = field->nextSibling;
                }

                // Field type
                switch (fieldSymbol->typ) {
                    case SYM_CHAR:
                        return makeCharSymbol('a');
                        break;
                    default:
                        return makeIntSymbol(0);
                }
                
            } else { // Variables & functions
                Node *idNode = n->firstChild;
                Symbol *s = lookup(table, idNode->value.val_str);
                if (!s) {
                    printf("Error line %d : variable %s undeclared\n",
                        idNode->lineno ,idNode->value.val_str);
                    semanticErrorCount++;

                    return makeIntSymbol(0);
                }
                return *s;
            }
        }
        // Function call
        case appelFonct: {
            Node *functionName = n->firstChild;
            Node *arguments = functionName->nextSibling;

            // Check for function
            Symbol *s = lookupFunction(table, functionName->value.val_str);
            if (!s) {
                printf("Error line %d : function %s undeclared\n",
                       functionName->lineno ,functionName->value.val_str);
                semanticErrorCount++;
            
                return makeIntSymbol(0);
            }

            // Check arguments
            if (s->value_funct.numberParams > numberArgs(arguments)) {
                printf("Error line %d : not enough arguments of function %s, %d expected, %d received\n",
                       functionName->lineno ,functionName->value.val_str,
                        s->value_funct.numberParams, numberArgs(arguments));
                semanticErrorCount++;

                return makeIntSymbol(0);

            } else if (s->value_funct.numberParams < numberArgs(arguments)) {
                  printf("Error line %d : too many arguments of function %s, %d expected, %d received\n",
                       functionName->lineno, functionName->value.val_str,
                        s->value_funct.numberParams, numberArgs(arguments));
                semanticErrorCount++;

                return makeIntSymbol(0);
            }
            
            // Check arguments
            int i = 0;
            for (Node *arg = arguments->firstChild; arg; arg = arg->nextSibling) {
                Symbol a = handleEval(arg, table);
                if (!castCheck(s->value_funct.paramTypes[i], a.typ)) {
                    printf("Error line %d : forbidden conversion of argument from function %s\n",
                        functionName->lineno, functionName->value.val_str);
                    semanticErrorCount++;

                    return makeIntSymbol(0);
                }
                i++;
            }

            return *s;
        }
        default:
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
        printf("Warning line %d : forbidden conversion of return value of function %s\n",
            n->lineno, functionName->value.val_str);
    }
}

static void handleAssign(Node *n, Node *instr, HashTable *table) {
    /*
    Handles an assign case from the AST, to the symbol tree.
    */
    Node *lhs = instr->firstChild;
    Node *rhs = lhs->nextSibling;

    Node *lastField = NULL; // Variable name

    // Get RValue expression
    Symbol value = handleEval(rhs, n->symTable);
    TypeValue typeValue; // Variable value
    

    // Get LValue variable
    if (numberArgs(lhs) > 1) { // Structure field
        Node *baseVar = lhs->firstChild;
        HashSEntry *baseStruct = lookupStructureVariable(table, baseVar->value.val_str);
        if (!baseStruct) {
            printf("Error line %d : structure %s undeclared\n",
                instr->lineno, baseVar->value.val_str);
            semanticErrorCount++;
            return;
        }

        StructDef *currentStruct = lookupField(table, baseStruct->symbol.structName);
        if (!currentStruct) {
            printf("Internal error : structure %s not found\n",
                baseStruct->symbol.structName);
            return;
        }
        
        // Next field (ex : p.a; ou p.color.r;)
        Node *field = baseVar->nextSibling;
        StructEntry *fieldSymbol = NULL;

        while (field) {
            fieldSymbol = lookupEntry(currentStruct, field->value.val_str);
            if (!fieldSymbol) {
                printf("Error line %d : field %s not found\n",
                    instr->lineno, field->value.val_str);
                semanticErrorCount++;
                return;
            }

            // If nested field (ex : p.color.r) : must be a structure
            if (field->nextSibling) {
                if (!lookupStruct(table, fieldSymbol->structName)) {
                    printf("Error line %d : nested structure %s not found\n",
                        instr->lineno, field->value.val_str);
                    semanticErrorCount++;
                    return;
                }
                currentStruct = lookupField(table, fieldSymbol->structName);
            }

            lastField = field;
            field = field->nextSibling;
        }

        typeValue = fieldSymbol->typ;

    } else { // Variable (int or char)
        Node *variableName = lhs->firstChild;
        lastField = variableName;

        // Get destination variable
        Symbol *varDest = lookup(n->symTable, variableName->value.val_str);
        if (!varDest) {
            printf("Error line %d : variable %s undeclared\n",
                instr->lineno, variableName->value.val_str);
            semanticErrorCount++;
            return;
        }

        typeValue = varDest->typ;
    }

    // Check variable type
    if (!castCheck(typeValue, value.typ)) {
        printf("Warning line %d : forbidden conversion of variable %s (int -> char)\n",
            instr->lineno, lastField->value.val_str);
        return;
    }
    
    // Check function type
    if ((value.typ == SYM_FUNCTION || value.typ == SYM_BUILTIN)) {
        // Assign with a void type function
        if (value.value_funct.returnType == SYM_NONE) {
            printf("Error line %d : variable %s assigned with incompatible type 'void'\n",
                instr->lineno, lastField->value.val_str);
            semanticErrorCount++;
            return;
        // Assign a char with an int
        } else if (value.value_funct.returnType == SYM_INT && typeValue == SYM_CHAR) {
            printf("Warning line %d : forbidden conversion of variable %s (int -> char)\n",
                instr->lineno, lastField->value.val_str);
            return;
        }
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

    int hasReturn = 0;

    // Check first if the function is a refedinition
    if (lookupFunction(table, functName->value.val_str)) {
        printf("Error line %d : function %s already declared\n",
            n->lineno, functName->value.val_str);
        semanticErrorCount++;
        return;
    }
    if (lookup(table, functName->value.val_str)) {
        printf("Error line %d : redefinition of the namespace %s\n",
            n->lineno, functName->value.val_str);
        semanticErrorCount++;
        return;
    }

    // Creates local symbol table for the function
    n->symTable = calloc(1, sizeof(HashTable));
    initHashTable(n->symTable, table, functName->value.val_str, 0);

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

    // Variable declaration in the function
    if (declVars) {
        handleDeclVars(declVars, n->symTable);
    }
    // Instructions in the function
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
                    // Check if it's indeed a void function
                    if (f.value_funct.returnType != SYM_NONE) {
                        printf("Error line %d : Non-void function %s should return a value\n",
                            instr->lineno, functName->value.val_str);
                        semanticErrorCount++;
                    }
                    break;
                }
                case returnSt: {
                     // Check if it's not a void function
                    if (f.value_funct.returnType == SYM_NONE) {
                        printf("Error line %d : Void function %s shouldn't return a value\n",
                            instr->lineno, functName->value.val_str);
                        semanticErrorCount++;
                    } else {
                        handleReturnType(instr, functName, f, n->symTable);
                        hasReturn = 1;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    if (f.value_funct.returnType != SYM_NONE) {
        if (!hasReturn) {
            printf("Error line %d : Non-void function %s doesn't return a value\n",
                n->lineno, functName->value.val_str);
            semanticErrorCount++;
        }
    }
}

static void checkMain(HashTable *table) {
    /*
    Checks if the 'main' function exists.
    */
    Symbol *main = lookupFunction(table, "main");
    if (!main){
        printf("Error : no 'main' function found\n");
        semanticErrorCount++;
        return;
    }
    if (main && main->value_funct.returnType != SYM_INT) {
        printf("Error : 'main' function should return an int\n");
        semanticErrorCount++;
    }
}

int buildSymbolTables(Node *n, HashTable *table){
    /*
    Builds all symbol tables, by linking them to the current node.
    It starts from the prog node.
    */
    if (!n) return 1;

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

    checkMain(table);
    return semanticErrorCount ? 0 : 1; // Check if there is no semantic error count
}