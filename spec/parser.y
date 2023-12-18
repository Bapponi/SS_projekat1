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

%token <ident> ONE_WORD_INST TWO_WORD_INST
%token <ident> THREE_WORD_INST FOUR_WORD_INST CALL_JUMP
%token <ident> STRING OPR_STRING IDENT LABEL CSR_REG GPR_REG
%token <num> OPR_HEX HEX OPR_DEC DEC
%token GLOBAL EXTERN SECTION WORD SKIP END LD ST CSRRD 
%token CSRWR PLUS MINUS LPARREN RPARREN SEMI COMMA ENDL

%%

program: directiveList END

directiveList: directive directiveList 
             | /* epsilon */

directive: extern | global | section

extern: EXTERN symbolList

global: GLOBAL symbolList

symbolList: IDENT COMMA symbolList
          | IDENT

section: SECTION IDENT sectionList

sectionList: sectionPart sectionList
           | sectionPart

sectionPart: labelSection | global

labelSection: LABEL instructionList

instructionList: instruction instructionList
               | /* epsilon */

instruction: ONE_WORD_INST
           | TWO_WORD_INST twoWord
           | THREE_WORD_INST threeWord
           | FOUR_WORD_INST fourWord
           | CALL_JUMP literal
           | LD ldPart
           | ST stPart
           | CSRRD csrrdPart
           | CSRWR csrwrPart
           | WORD symbolLiteralList
           | SKIP literal

twoWord: GPR_REG

threeWord: GPR_REG COMMA GPR_REG

fourWord: GPR_REG COMMA GPR_REG COMMA operand

ldPart: operand COMMA GPR_REG

stPart: GPR_REG COMMA operand

csrrdPart: CSR_REG COMMA GPR_REG

csrwrPart: GPR_REG COMMA CSR_REG

operand: OPR_DEC | OPR_HEX | OPR_STRING

symbolLiteralList: literal COMMA symbolLiteralList
                 | literal

literal: DEC | HEX | IDENT

%%

int yyerror(const char* msg) {
    fprintf(stderr, "Parser ERROR: %s on line %d\n", msg, lineNumber);
    exit(EXIT_FAILURE);
}

int main() {
    yyparse();
    return 0;
}
