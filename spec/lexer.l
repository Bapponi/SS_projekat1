%{
#include "parser.tab.h"
//#include "parser.h"
//#include "./inc/assembler.hpp"
int lineNumber=1;
%}

%option noyywrap
%option nodefault


THREE_WORD_INSTRUCT     (xchg|add|sub|mul|div|and|or|xor|shl|shr)[ \t]        
OPR_STR                 \$([A-Za-z_][A-Za-z0-9_]*)
IDENTIFICATOR           [A-Za-z_][A-Za-z0-9_]*
LAB                     [A-Za-z_][A-Za-z0-9_]*:
CSR_REGISTER            "%"(status|handler|cause)
GPR_REGISTER            "%"(r(0|[1-9]|1[0-5])|sp|pc)
OPR_HEXA                \$0[Xx][0-9A-Fa-f]+
HEXA                    0[Xx][0-9A-Fa-f]+
OPR_DECI                \$(0|[1-9][0-9]*)
DECI                    (0|[1-9][0-9]*)
BADIDENT                [0-9][A-Za-z0-9_]*


%%
\.global[ \t]           { printf("GLOBAL\n");
                          return GLOBAL; }
\.extern[ \t]           { printf("EXTERN\n");
                          return EXTERN; }
\.section[ \t]          { printf("SECTION\n");
                          return SECTION; }
\.word[ \t]             { printf("WORD\n");
                          return WORD; }  
\.skip[ \t]             { printf("SKIP\n");
                          return SKIP; }
\.end                   { lineNumber=1;
                          printf("END\n");
                          return END; }  

(halt|int|iret|ret)     { yylval.directive = strdup(yytext);
                          printf("ONE_WORD_INST\n");
                          return ONE_WORD_INST; }
(push|pop|not)[ \t]     { yylval.directive = strdup(yytext);
                          printf("TWO_WORD_INST\n");
                          return TWO_WORD_INST; }
{THREE_WORD_INSTRUCT}   { yylval.directive = strdup(yytext);
                          printf("THREE_WORD_INST\n");
                          return THREE_WORD_INST; }
(beq|bne|bgt)[ \t]      { yylval.directive = strdup(yytext);
                          printf("FOUR_WORD_INST\n");
                          return FOUR_WORD_INST; }
(jmp|call)[ \t]         { yylval.directive = strdup(yytext);
                          printf("CALL_JUMP\n");
                          return CALL_JUMP; }
ld[ \t]                 { printf("LD\n");
                          return LD; }
st[ \t]                 { printf("ST\n");
                          return ST; }
csrrd[ \t]              { printf("CSRRD\n");
                          return CSRRD; }
csrwr[ \t]              { printf("CSRWR\n");
                          return CSRWR; }

"+"                     { printf("PLUS\n");
                          return PLUS; }
"-"                     { printf("MINUS\n");
                          return MINUS; }
"["                     { printf("LPARREN\n");
                          return LPARREN; }
"]"                     { printf("RPARREN\n");
                          return RPARREN; }
";"                     { printf("SEMI\n");
                          return SEMI; }
","                     { printf("COMMA\n");
                          return COMMA; }
"\n"                    { ++lineNumber;
                          printf("ENDL\n");
                          return ENDL; }

"#"[^\n\r]*             { printf("komentar\n");
                          /* skips comments */ }
[ \t]*                  { printf("prazno\n");
                          /* whitespace */ }
\"(.+|\n+)\"            { yylval.ident = strdup(yytext);
                          printf("STRING\n");
                          return STRING; }
{OPR_STR}               { yylval.ident = strdup(yytext);
                          printf("OPR_STRING\n");
                          return OPR_STRING; }
{IDENTIFICATOR}         { yylval.ident = strdup(yytext);
                          printf("IDENT\n");
                          return IDENT; }
{LAB}                   { yylval.ident = strdup(yytext);
                          printf("LABEL\n");
                          return LABEL; }
{CSR_REGISTER}          { yylval.ident = strdup(yytext);
                          printf("CSR_REG\n");
                          return CSR_REG; }
{GPR_REGISTER}          { yylval.ident = strdup(yytext);
                          printf("GPR_REG\n");
                          return GPR_REG; }
{OPR_HEXA}              { sscanf(yytext, "%x", &yylval.num);
                          printf("OPR_HEX\n");
                          return OPR_HEX; }
{HEXA}                  { sscanf(yytext, "%x", &yylval.num);
                          printf("HEX\n");
                          return HEX; }
{OPR_DECI}              { sscanf(yytext, "%d", &yylval.num); 
                          printf("OPR_DEC\n");
                          return OPR_DEC; }
{DECI}                   { sscanf(yytext, "%d", &yylval.num); 
                          printf("DEC\n");
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