%{
#include "parser.tab.h"
int lineNum=1;
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


%%
\.global[ \t]           { yylval.string = strdup(yytext);
                          printf(" GLOBAL");
                          return GLOBAL; }
\.extern[ \t]           { yylval.string = strdup(yytext);
                          printf(" EXTERN");
                          return EXTERN; }
\.section[ \t]          { yylval.string = strdup(yytext);
                          printf(" SECTION");
                          return SECTION; }
\.word[ \t]             { yylval.string = strdup(yytext);
                          printf(" WORD");
                          return WORD; }  
\.skip[ \t]             { yylval.string = strdup(yytext);
                          printf(" SKIP");
                          return SKIP; }
\.end                   { yylval.string = strdup(yytext);
                          lineNum=1;
                          printf(" END");
                          return END; }  

(halt|int|iret|ret)     { yylval.string = strdup(yytext);
                          printf(" ONE_WORD_INST");
                          return ONE_WORD_INST; }
(push|pop|not)[ \t]     { yylval.string = strdup(yytext);
                          printf(" TWO_WORD_INST");
                          return TWO_WORD_INST; }
{THREE_WORD_INSTRUCT}   { yylval.string = strdup(yytext);
                          printf(" THREE_WORD_INST");
                          return THREE_WORD_INST; }
(beq|bne|bgt)[ \t]      { yylval.string = strdup(yytext);
                          printf(" FOUR_WORD_INST");
                          return FOUR_WORD_INST; }
(jmp|call)[ \t]         { yylval.string = strdup(yytext);
                          printf(" CALL_JUMP");
                          return CALL_JUMP; }
ld[ \t]                 { yylval.string = strdup(yytext);
                          printf(" LD");
                          return LD; }
st[ \t]                 { yylval.string = strdup(yytext);
                          printf(" ST");
                          return ST; }
csrrd[ \t]              { yylval.string = strdup(yytext);
                          printf(" CSRRD");
                          return CSRRD; }
csrwr[ \t]              { yylval.string = strdup(yytext);
                          printf(" CSRWR");
                          return CSRWR; }

"+"                     { yylval.string = strdup(yytext);
                          printf(" PLUS");
                          return PLUS; }
"-"                     { yylval.string = strdup(yytext);
                          printf(" MINUS");
                          return MINUS; }
"["                     { yylval.string = strdup(yytext);
                          printf(" LPARREN");
                          return LPARREN; }
"]"                     { yylval.string = strdup(yytext);
                          printf(" RPARREN");
                          return RPARREN; }
";"                     { yylval.string = strdup(yytext);
                          printf(" SEMI");
                          return SEMI; }
","                     { yylval.string = strdup(yytext);
                          printf(" COMMA");
                          return COMMA; }


"#"[^\n\r]*             { printf(" komentar");
                          /* skips comments */ }
[ \t]*                  { printf(" ");
                          /* whitespace */ }
[ \n]                  { printf("\n");
                          lineNum++;
                          /* new row */ }                          
\"(.+|\n+)\"            { yylval.string = strdup(yytext);
                          printf(" STRING");
                          return STRING; }
{OPR_STR}               { yylval.string = strdup(yytext);
                          printf(" OPR_STRING");
                          return OPR_STRING; }
{CSR_REGISTER}          { yylval.string = strdup(yytext);
                          printf(" CSR_REG");
                          return CSR_REG; }
{GPR_REGISTER}          { yylval.string = strdup(yytext);
                          printf(" GPR_REG");
                          return GPR_REG; }
{OPR_HEXA}              { //sscanf(yytext, "%x", &yylval.num);
                          yylval.string = strdup(yytext);
                          printf(" OPR_HEX");
                          return OPR_HEX; }
{HEXA}                  { //sscanf(yytext, "%x", &yylval.num);
                          yylval.string = strdup(yytext);
                          printf(" HEX");
                          return HEX; }
{OPR_DECI}              { //sscanf(yytext, "%d", &yylval.num); 
                          yylval.string = strdup(yytext);
                          printf(" OPR_DEC");
                          return OPR_DEC; }
{DECI}                   { //sscanf(yytext, "%d", &yylval.num); 
                           yylval.string = strdup(yytext);
                           printf(" DEC");
                           return DEC; }
{LAB}                   { yylval.string = strdup(yytext);
                          printf(" LABEL");
                          return LABEL; }
{IDENTIFICATOR}         { yylval.string = strdup(yytext);
                          printf(" IDENT");
                          return IDENT; }

.                       { printf("ERROR: UNKNOWN LEXER ELEMENT: %s", yytext);
                          return 1;}
%%

int
yyerror(const char* msg)
{
    fprintf(stderr, "Lexer ERROR: %s on line %d\n", msg, lineNum);
    exit(EXIT_FAILURE);
}