%{
#include "parser.h"
#include "./inc/assembler.hpp"
int lineNumber=1;
%}

%option outfile="lexer.c" header-file="lexer.h"

%option noyywrap
%option nodefault


SYMBOL_MIGR             \.(extern|global)[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*([ \t]*,[ \t]*[A-Za-z_][A-Za-z0-9_]*)*        
THREE_WORD_INSTRUCT     (xchg|add|sub|mul|div|and|or|xor|shl|shr)[ \t]        
OPR_STR                 \$([A-Za-z_][A-Za-z0-9_]*)
IDENTIFICATOR           [A-Za-z_][A-Za-z0-9_]*
LAB                     [A-Za-z_][A-Za-z0-9_]*:
SIS_REGISTER            "%"(status|handler|cause)
REGISTER                "%"(r(0|[1-9]|1[0-5])|sp|pc)
OPR_HEXA                \$0[Xx][0-9A-Fa-f]+
HEXA                    0[Xx][0-9A-Fa-f]+
OPR_DECI                \$(0|[1-9][0-9]*)
DECI                    (0|[1-9][0-9]*)
BADIDENT                  [0-9][A-Za-z0-9_]*


%%
{SYMBOL_MIGR}           { yylval.directive = strdup(yytext);
                          return SYMBOL_MIGRATION; }
\.section[ \t]          { return SECTION; }
\.word[ \t]             { return WORD; }  
\.skip[ \t]             { return SKIP; }
\.end                   { lineNumber=1;
                          return END; }  

(halt|int|iret|ret)     { yylval.directive = strdup(yytext);
                          return ONE_WORD_INST; }
(push|pop|not)[ \t]     { yylval.directive = strdup(yytext);
                          return TWO_WORD_INST; }
{THREE_WORD_INSTRUCT}   { yylval.directive = strdup(yytext);
                          return THREE_WORD_INST; }
(beq|bne|bgt)[ \t]      { yylval.directive = strdup(yytext);
                          return FOUR_WORD_INST; }
(jmp|call)[ \t]         { yylval.directive = strdup(yytext);
                          return JMP_CALL; }
ld[ \t]                 { return LD; }
st[ \t]                 { return ST; }
csrrd[ \t]              { return CSRRD; }
csrwr[ \t]              { return CSRWR; }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"["                     { return LPARREN; }
"]"                     { return RPARREN; }
";"                     { return SEMI; }
","                     { return COMMA; }
"\n"                    { ++lineNumber;
                          return ENDL; }

"#"[^\n\r]*             { /* skips comments */ }
[ \t]*                  { /* whitespace */ }
\"(.+|\n+)\"            { yylval.ident = strdup(yytext);
                          return STRING; }
{OPR_STR}               { yytext++;yylval.ident = strdup(yytext); 
                          return OPR_STRING; }
{IDENTIFICATOR}         { yylval.ident = strdup(yytext);
                          return IDENT; }
{LAB}                   { yylval.ident = strdup(yytext);
                          return LABEL; }
{SIS_REGISTER}          { yylval.ident = strdup(yytext);
                          return SIS_REG; }
{REGISTER}              { yylval.ident = strdup(yytext);
                          return REG; }
{OPR_HEXA}              { yytext++;
                          sscanf(yytext, "%x", &yylval.num); 
                          return OPR_HEX; }
{HEXA}                  { sscanf(yytext, "%x", &yylval.num);
                          return HEX; }
{OPR_DECI}              { yytext++;
                          sscanf(yytext, "%d", &yylval.num); 
                          return OPR_DEC; }
{DECI}                   { sscanf(yytext, "%d", &yylval.num); 
                          return DEC; }

{BADIDENT}              { printf("BAD IDENTIFICATOR!!!\n"); 
                          return 1; }
.                       { printf("UNKNOWN TOKEN: %s\n", yytext);
                          return 1;}
%%

int
yyerror(const char* msg)
{
    fprintf(stderr, "Lexer ERROR: %s on line %d\n", msg, lineNumber);
    exit(EXIT_FAILURE);
}