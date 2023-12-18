%{
#include "parser.h"
#include "./inc/assembler.hpp"

extern int yylex();   // Bison needs to know how to call the lexer
extern char* yytext;  // Bison needs to know about yytext

int lineNumber = 1;   // Keep track of the line number

%}

%union {
    int num;
    char* ident;
}

%token <ident> SYMBOL_MIGRATION ONE_WORD_INST TWO_WORD_INST
%token <ident> THREE_WORD_INST FOUR_WORD_INST JMP_CALL
%token <ident> STRING OPR_STRING IDENT LABEL SIS_REG REG
%token <num> OPR_HEX HEX OPR_DEC DEC
%token SECTION WORD SKIP END LD ST CSRRD CSRWR PLUS
%token MINUS LPARREN RPARREN SEMI COMMA ENDL

%%

program: statements

statements: /* empty */
         | statements statement

statement: directive ENDL
         | instruction ENDL
         | LABEL instruction ENDL

directive: SYMBOL_MIGRATION IDENT
         | SECTION
         | WORD
         | SKIP
         | END
         | OPR_STRING STRING
         | IDENT OPR_STRING STRING

instruction: ONE_WORD_INST
           | TWO_WORD_INST
           | THREE_WORD_INST
           | FOUR_WORD_INST
           | JMP_CALL
           | LD
           | ST
           | CSRRD
           | CSRWR

%%

int yyerror(const char* msg) {
    fprintf(stderr, "Parser ERROR: %s on line %d\n", msg, lineNumber);
    exit(EXIT_FAILURE);
}

int main() {
    yyparse();
    return 0;
}
