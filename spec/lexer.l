%{
//#include <iostream>
//#include <fstream>
//#include <string>
#include "parser.tab.h"
//#include "parser.h"
//#include "./inc/assembler.hpp"
int lineNumber=1;
//extern int token;

%}

%option noyywrap
%option nodefault


THREE_WORD_INSTRUCT     (xchg|add|sub|mul|div|and|or|xor|shl|shr)[ \t]        
OPR_STR                 \$([A-Za-z_][A-Za-z0-9_]*)
CSR_REGISTER            "%"(status|handler|cause)
GPR_REGISTER            "%"(r(0|[1-9]|1[0-5])|sp|pc)
OPR_HEXA                \$0[Xx][0-9A-Fa-f]+
HEXA                    0[Xx][0-9A-Fa-f]+
OPR_DECI                \$(0|[1-9][0-9]*)
DECI                    (0|[1-9][0-9]*)
LAB                     [A-Za-z_][A-Za-z0-9_]*:
IDENTIFICATOR           [A-Za-z_][A-Za-z0-9_]*
BADIDENT                [0-9][A-Za-z0-9_]*


%%
\.global[ \t]           { printf(" GLOBAL");
                          //token = 1;
                          return GLOBAL; }
\.extern[ \t]           { printf(" EXTERN");
                          return EXTERN; }
\.section[ \t]          { printf(" SECTION");
                          return SECTION; }
\.word[ \t]             { printf(" WORD");
                          return WORD; }  
\.skip[ \t]             { printf(" SKIP");
                          return SKIP; }
\.end                   { lineNumber=1;
                          printf(" END");
                          return END; }  

(halt|int|iret|ret)     { yylval.directive = strdup(yytext);
                          printf(" ONE_WORD_INST");
                          return ONE_WORD_INST; }
(push|pop|not)[ \t]     { yylval.directive = strdup(yytext);
                          printf(" TWO_WORD_INST");
                          return TWO_WORD_INST; }
{THREE_WORD_INSTRUCT}   { yylval.directive = strdup(yytext);
                          printf(" THREE_WORD_INST");
                          return THREE_WORD_INST; }
(beq|bne|bgt)[ \t]      { yylval.directive = strdup(yytext);
                          printf(" FOUR_WORD_INST");
                          return FOUR_WORD_INST; }
(jmp|call)[ \t]         { yylval.directive = strdup(yytext);
                          printf(" CALL_JUMP");
                          return CALL_JUMP; }
ld[ \t]                 { printf(" LD");
                          return LD; }
st[ \t]                 { printf(" ST");
                          return ST; }
csrrd[ \t]              { printf(" CSRRD");
                          return CSRRD; }
csrwr[ \t]              { printf(" CSRWR");
                          return CSRWR; }

"+"                     { printf(" PLUS");
                          return PLUS; }
"-"                     { printf(" MINUS");
                          return MINUS; }
"["                     { printf(" LPARREN");
                          return LPARREN; }
"]"                     { printf(" RPARREN");
                          return RPARREN; }
";"                     { printf(" SEMI");
                          return SEMI; }
","                     { printf(" COMMA");
                          return COMMA; }


"#"[^\n\r]*             { printf(" komentar");
                          /* skips comments */ }
[ \t]*                  { printf(" ");
                          /* whitespace */ }
[ \n]                  { printf("\n");
                          lineNumber++;
                          /* new row */ }                          
\"(.+|\n+)\"            { yylval.ident = strdup(yytext);
                          printf(" STRING");
                          return STRING; }
{OPR_STR}               { yylval.ident = strdup(yytext);
                          printf(" OPR_STRING");
                          return OPR_STRING; }
{CSR_REGISTER}          { yylval.ident = strdup(yytext);
                          printf(" CSR_REG");
                          return CSR_REG; }
{GPR_REGISTER}          { yylval.ident = strdup(yytext);
                          printf(" GPR_REG");
                          return GPR_REG; }
{OPR_HEXA}              { sscanf(yytext, "%x", &yylval.num);
                          printf(" OPR_HEX");
                          return OPR_HEX; }
{HEXA}                  { sscanf(yytext, "%x", &yylval.num);
                          printf(" HEX");
                          return HEX; }
{OPR_DECI}              { sscanf(yytext, "%d", &yylval.num); 
                          printf(" OPR_DEC");
                          return OPR_DEC; }
{DECI}                   { sscanf(yytext, "%d", &yylval.num); 
                          printf(" DEC");
                          return DEC; }
{LAB}                   { yylval.ident = strdup(yytext);
                          printf(" LABEL");
                          return LABEL; }
{IDENTIFICATOR}         { yylval.ident = strdup(yytext);
                          printf(" IDENT");
                          return IDENT; }


{BADIDENT}              { printf(" BAD IDENTIFICATOR!!!"); 
                          return 1; }
.                       { printf(" UNKNOWN TOKEN: %s", yytext);
                          return 1;}
%%

int
yyerror(const char* msg)
{
    fprintf(stderr, "Lexer ERROR: %s on line %d\n", msg, lineNumber);
    exit(EXIT_FAILURE);
}