%{
/* tpc-2025-2026.lex */
#include "tree.h"
#include "tpc-2025-2026.tab.h" // Définition des token
%}
%x COMMENTAIRE
NUM [0-9]+
IDENT [a-zA-Z_][a-zA-Z0-9_]*
%option nounput
%option noinput
%option noyywrap
%option yylineno
%%
"/*" {BEGIN COMMENTAIRE;}
<COMMENTAIRE>"*/" {BEGIN INITIAL;}
<COMMENTAIRE>.|\n;
"//".* ;

"int"                         { strcpy(yylval.type, "int"); return TYPE; }
"char"                        { strcpy(yylval.type, "char"); return TYPE; }

"if"                          return IF;
"else"                        return ELSE;
"while"                       return WHILE;
"return"                      return RETURN;
"void"                        return VOID;
"struct"                      return STRUCT;

\'([^\\\']|\\.)\'             { yylval.byte = yytext[1]; return CHARACTER; }
"+"                           { yylval.byte = '+'; return ADDSUB; }
"-"                           { yylval.byte = '-'; return ADDSUB; }
"*"                           { yylval.byte = '*'; return DIVSTAR; }
"/"                           { yylval.byte = '/'; return DIVSTAR; }
"%"                           { yylval.byte = '%'; return DIVSTAR; }

{NUM}                         { yylval.num = atoi(yytext); return NUM; }

{IDENT}                       { strcpy(yylval.ident, yytext); return IDENT; }

"<="                          { strcpy(yylval.comp, yytext); return ORDER; }
">="                          { strcpy(yylval.comp, yytext); return ORDER; }
"<"                           { strcpy(yylval.comp, yytext); return ORDER; }
">"                           { strcpy(yylval.comp, yytext); return ORDER; }
"=="                          { strcpy(yylval.comp, yytext); return EQ; }
"!="                          { strcpy(yylval.comp, yytext); return EQ; }

"||"                          return OR;
"&&"                          return AND;
"!"                           return '!';
"("                           return '(';
")"                           return ')';
","                           return ',';
";"                           return ';';
"="                           return '=';
"."                           return '.';
"{"                           return '{';
"}"                           return '}';

[ \r\t]+  ;
\n ;
.                             return yytext[0];
%%