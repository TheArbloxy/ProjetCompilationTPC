%{
/* tpc-2025-2026.y */
/* Syntaxe en TPC */
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "tree.h"
#include "nasm_handler.h"
#include "symbol_table.h"
#include "symbol_handler.h"
extern int yylineno;
extern char * yytext;
int yylex();
void yyerror(char *);
Node *node = NULL;
%}
%union {
    char byte;
    int num;
    char ident[256];
    char type[10];
    char comp[3];
    Node *node;
}

%token <byte> CHARACTER ADDSUB DIVSTAR
%token <num> NUM
%token <ident> IDENT
%token <type> TYPE
%token <comp> ORDER EQ
%token OR AND IF ELSE WHILE RETURN VOID STRUCT

%nonassoc IFX
%nonassoc ELSE

%type <node> Prog DeclVars DeclFoncts DeclFonct Declarateurs EnTeteFonct Corps CorpsStruct DeclStruct Parametres ListTypVar Exp TB FB M E T F ListExp Arguments Instr SuiteInstr FieldAccess

%%
Prog:  DeclVars DeclFoncts { node = makeNode(prog);
                             addChild(node, $1);
                             addChild(node, $2);
                           }                           
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';' { Node *decl = makeNode(declVar);
                                        if (strcmp($2, "int") == 0) addChild(decl, makeNode(typeInt));
                                        else if (strcmp($2, "char") == 0) addChild(decl, makeNode(typeChar));
                                        addChild(decl, $3);

                                        addChild($1, decl);
                                        $$ = $1;
                                        }
    |  DeclVars STRUCT IDENT Declarateurs ';' { Node *decl = makeNode(declVarStruct);

                                                addChild(decl, makeNode(typeStruct));
                                                addChild(decl, makeNodeString(id, $3));
                                                addChild(decl, $4);
                                                addChild($1, decl);

                                                $$ = $1;
                                                }
    |  DeclVars STRUCT IDENT CorpsStruct ';' { Node *decl = makeNode(declVarStruct);

                                                      addChild(decl, makeNode(typeStruct));
                                                      addChild(decl, makeNodeString(id, $3));
                                                      addChild(decl, $4);
                                                      addChild($1, decl);

                                                      $$ = $1;
                                                      }
    | { $$ = makeNode(declVars); }
    ;
Declarateurs:
       Declarateurs ',' IDENT { addChild($1, makeNodeString(id, $3));
                                $$ = $1;
                                }
    |  IDENT { Node *n = makeNode(declarateurs); 
               addChild(n, makeNodeString(id, $1));
               $$ = n;
               }
    ;
DeclFoncts:
       DeclFoncts DeclFonct { addChild($1, $2);
                              $$ = $1;
                             }
    |  DeclFonct { Node *n = makeNode(declFoncts); 
                   addChild(n, $1);
                   $$ = n;
                   }
    ;
DeclFonct:
       EnTeteFonct Corps { Node *n = makeNode(declFonct); 
                           addChild(n, $1);
                           addChild(n, $2);
                           $$ = n;
                           }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')' { Node *n = makeNode(enTete); 
                                       if (strcmp($1, "int") == 0) addChild(n , makeNode(typeInt));
                                       else if (strcmp($1, "char") == 0) addChild(n , makeNode(typeChar));
                                       addChild(n, makeNodeString(id, $2));
                                       if ($4 != NULL) addChild(n, $4);
                                       $$ = n;
                                       }
    |  STRUCT IDENT IDENT '(' Parametres ')' { Node *n = makeNode(enTete); 
                                         addChild(n, makeNode(typeStruct));
                                         addChild(n, makeNodeString(id, $2));
                                         addChild(n, makeNodeString(id, $3));
                                         if ($5 != NULL) addChild(n, $5);
                                         $$ = n;
                                         }
    |  VOID IDENT '(' Parametres ')' { Node *n = makeNode(enTete); 
                                       addChild(n, makeNode(typeVoid));
                                       addChild(n, makeNodeString(id, $2));
                                       if ($4 != NULL) addChild(n, $4);
                                       $$ = n;
                                       }
    ;
Parametres:
       VOID { Node *n = makeNode(Parametres);
              addChild(n, makeNode(typeVoid));
              $$ = n; 
              }
    |  ListTypVar { Node *n = makeNode(Parametres);
                    addChild(n, $1);
                    $$ = n;
                    }
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT { Node *n = makeNode(TypVar);
                                   if (strcmp($3, "int") == 0) addChild(n , makeNode(typeInt));
                                   else if (strcmp($3, "char") == 0) addChild(n , makeNode(typeChar));
                                   addChild(n, makeNodeString(id, $4));

                                   addChild($1, n);
                                   $$ = $1;
                                   }
    |  ListTypVar ',' STRUCT IDENT IDENT { Node *n = makeNode(TypVar);
                                     addChild(n, makeNode(typeStruct)); 
                                     addChild(n, makeNodeString(id, $4));
                                     addChild(n, makeNodeString(id, $5));

                                     addChild($1, n);
                                     $$ = $1;
                                     }
    |  TYPE IDENT { Node *n = makeNode(ListTypVar);
                    Node *param = makeNode(TypVar);

                    if (strcmp($1, "int") == 0) addChild(param, makeNode(typeInt));
                    else if (strcmp($1, "char") == 0) addChild(param, makeNode(typeChar));
                    addChild(param, makeNodeString(id, $2));

                    addChild(n, param);
                    $$ = n;
                    }
    |  STRUCT IDENT IDENT { Node *n = makeNode(ListTypVar);
                            Node *param = makeNode(TypVar);

                            addChild(param, makeNode(typeStruct)); 
                            addChild(param, makeNodeString(id, $2));
                            addChild(param, makeNodeString(id, $3));

                            addChild(n, param);
                            $$ = n;
                            }
    ;
Corps: '{' DeclVars SuiteInstr '}' { Node *n = makeNode(corps);
                                     addChild(n, $2);
                                     addChild(n, $3);
                                     $$ = n;
                                     }
    ;
CorpsStruct: '{' DeclStruct '}' { $$ = $2; }
    ;
DeclStruct:
    DeclStruct TYPE Declarateurs ';' { Node *n = makeNode(declChamps);
                                       if (strcmp($2, "int") == 0) addChild(n, makeNode(typeInt));
                                       else if (strcmp($2, "char") == 0) addChild(n, makeNode(typeChar));
                                       addChild(n, $3);

                                       addChild($1, n);
                                       $$ = $1;
                                       }
|  DeclStruct STRUCT IDENT Declarateurs ';' { Node *n = makeNode(declChamps);
                                              addChild(n, makeNode(typeStruct));
                                              addChild(n, makeNodeString(id, $3));
                                              addChild(n, $4);

                                              addChild($1, n);

                                              $$ = $1;
                                            }
 | TYPE Declarateurs ';' { Node *n = makeNode(declStruct);
                           Node *param = makeNode(declChamps);

                           if (strcmp($1, "int") == 0) addChild(param, makeNode(typeInt));
                           else if (strcmp($1, "char") == 0) addChild(param, makeNode(typeChar));
                           addChild(param, $2);

                           addChild(n, param);
                           $$ = n;
                           }
 | STRUCT IDENT Declarateurs ';' { Node *n = makeNode(declStruct);
                                   Node *param = makeNode(declChamps);
                                   addChild(n, makeNode(typeStruct));
                                   addChild(n, makeNodeString(id, $2));
                                   addChild(n, $3);

                                   addChild(n, param);
                                   $$ = n;
                                   }
;
SuiteInstr:
       SuiteInstr Instr { addChild($1, $2);
                          $$ = $1;
                          }
    | { $$ = makeNode(SuiteInstr); }
    ;
Instr:
        FieldAccess '=' Exp ';' { Node *n = makeNode(assign);
                                  addChild(n, $1);
                                  addChild(n, $3);
                                  $$ = n;
                                  }
    |  IF '(' Exp ')' Instr %prec IFX { Node *n = makeNode(ifSt);
                                        addChild(n, $3);
                                        addChild(n, $5);
                                        $$ = n;
                                        }
    |  IF '(' Exp ')' Instr ELSE Instr { Node *n = makeNode(elseSt);
                                         addChild(n, $3);
                                         addChild(n, $5);
                                         addChild(n, $7);
                                         $$ = n;
                                         }
    |  WHILE '(' Exp ')' Instr { Node *n = makeNode(whileSt);
                                 addChild(n, $3);
                                 addChild(n, $5);
                                 $$ = n;
                                 }
    |  IDENT '(' Arguments  ')' ';' { Node *n = makeNode(appelFonct);
                                      addChild(n, makeNodeString(id, $1));
                                      addChild(n, $3);
                                      $$ = n;
                                      }
    |  RETURN Exp ';' { Node *n = makeNode(returnSt);
                        addChild(n, $2);
                        $$ = n;
                        }
    |  RETURN ';' { Node *n = makeNode(voidSt);
                    $$ = n;
                    }
    |  '{' SuiteInstr '}' { $$ = $2; }
    |  ';' { $$ = NULL; }
    ;
Exp :  Exp OR TB { Node *n = makeNode(Exp); 
                   addChild(n, $1);
                   addChild(n, makeNode(orExp));
                   addChild(n, $3);
                   $$ = n;
                   }
    |  TB { $$ = $1; }
    ;
TB  :  TB AND FB { Node *n = makeNode(TB);
                   addChild(n, $1);
                   addChild(n, makeNode(andExp));
                   addChild(n, $3);
                   $$ = n;
                   }
    |  FB { $$ = $1; }
    ;
FB  :  FB EQ M  { Node *n = makeNode(FB);
                  addChild(n, $1);
                  if (strcmp($2, "==") == 0) addChild(n, makeNode(equals));
                  else if (strcmp($2, "!=") == 0) addChild(n, makeNode(notEquals));
                  addChild(n, $3);
                  $$ = n;
                  }
    |  M { $$ = $1; }
    ;
M   :  M ORDER E  { Node *n = makeNode(M);
                    addChild(n, $1);
                    if (strcmp($2, "<") == 0) addChild(n, makeNode(orderInf));
                    else if (strcmp($2, "<=") == 0) addChild(n, makeNode(orderInfEquals));
                    else if (strcmp($2, ">") == 0) addChild(n, makeNode(orderSup));
                    else if (strcmp($2, ">=") == 0) addChild(n, makeNode(orderSupEquals));
                    addChild(n, $3);
                    $$ = n;
                    }
    |  E { $$ = $1; }
    ;
E   :  E ADDSUB T { Node *n = makeNode(E);
                    addChild(n, $1);
                    if ($2 == '+') addChild(n, makeNode(add));
                    else          addChild(n, makeNode(sub));
                    addChild(n, $3);
                    $$ = n;
                    }
    |  T { $$ = $1; }
    ;    
T   :  T DIVSTAR F { Node *n = makeNode(T);
                     addChild(n, $1);
                     if ($2 == '*') addChild(n, makeNode(mul));
                     else if ($2 == '/') addChild(n, makeNode(divstar));
                     else addChild(n, makeNode(mod));
                     addChild(n, $3);
                     $$ = n;
                    }
    |  F { $$ = $1; }
    ;
F   :  ADDSUB F { Node *n;
                  if ($1 == '+') n = makeNode(unaryplus);
                  else          n = makeNode(unaryminus);
                  addChild(n, $2);
                  $$ = n;
                  }
    |  '!' F { Node *n = makeNode(notInstr);
               addChild(n, $2);
               $$ = n;
               }
    |  '(' Exp ')' { $$ = $2; }
    |  NUM { $$ = makeNodeInt(id, $1); }
    |  CHARACTER { $$ = makeNodeChar(character, $1); }
    |  FieldAccess { $$ = $1; }
    |  IDENT '(' Arguments  ')' { Node *n = makeNode(appelFonct);
                                  addChild(n, makeNodeString(id, $1)); 
                                  addChild(n, $3);
                                  $$ = n;
                                  }
    ;
Arguments:
       ListExp { $$ = $1; }
    | { $$ = makeNode(Arguments); }
    ;
ListExp:
       ListExp ',' Exp { addChild($1, $3);
                         $$ = $1;
                         }
    |  Exp { Node *n = makeNode(ListExp);
             addChild(n, $1);
             $$ = n;
             }
    ;
FieldAccess:
        FieldAccess '.' IDENT { addChild($1, makeNodeString(id, $3));
                                $$ = $1;
                                }
    |   IDENT { Node *n = makeNode(fieldAccess); 
                addChild(n, makeNodeString(id, $1));
                $$ = n; 
                }
    ;
%%
void yyerror(char* message){
    printf("Syntax error line %d near '%s' : %s\n", yylineno, yytext, message);
}

void helpprint(const char* exe) {
    char *helpdesc = "Prints a description of the user interface.";
    char *treedesc = "Prints the AST in the standard output.";
    fprintf(stdout, "Usage: %s [OPTIONS...] < fichier.tpc\n", exe);
    fprintf(stdout, "-h, --help %50s\n", helpdesc);
    fprintf(stdout, "-t, --tree %45s\n", treedesc);
}

int main(int argc, char **argv) {
    /* Command line parser */
    int opt;
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"tree",    no_argument,       0, 't'},
        {"symtabs", no_argument,       0, 's'},
        {0, 0, 0, 0}
    };

    /* Optional commands */
    int tree = 0;
    int symtabs = 0;
    while ((opt = getopt_long(argc, argv, "hts", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                helpprint(argv[0]);
                return 0;
            case 't':
                tree = 1;
                break;
            case 's':
                symtabs = 1;
                break;
            default:
                fprintf(stderr, "Unknown option.\n");
                return 2;
            }
    }

    /* AST Tree */
    if (yyparse() == 0) {

        FILE* f = fopen("_anonymous.asm", "w");
        if (!f) {
            printf("ouverture du fichier _anonymous.asm échouée");
            return 0;
        }
        
        /* Initiates and builds the symbol table */
        HashTable *global = calloc(1, sizeof(HashTable));
        initHashTable(global, NULL);
        addBuiltIns(global);
        buildSymbolTables(node, global);

        if (symtabs) {
            printAllTables(node);
        }

        /* Creates the _anonymous.asm file */
        parcoursArbre(node, f);
        fclose(f);

        if (node != NULL && tree == 1) {
            printTree(node);
        }
        deleteTree(node);
        fprintf(stdout, "Test passed.\n");
    } else {
        fprintf(stdout, "Test failed.\n");
    }

    return 0;
}
