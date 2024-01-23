%{
//#include "parser.h"
#include "../inc/assembler.hpp"
#include <stdio.h>

extern int yyparse(void);
extern int yyerror(const char *s);

extern int yylex();   // Bison needs to know how to call the lexer
extern char* yytext;  // Bison needs to know about yytext
%}

%union {
    int num;
    char* ident;
    char* directive;
}

%token <directive> ONE_WORD_INST TWO_WORD_INST
%token <directive> THREE_WORD_INST FOUR_WORD_INST CALL_JUMP
%token <directive> STRING OPR_STRING IDENT LABEL CSR_REG GPR_REG
%token <num> OPR_HEX HEX OPR_DEC DEC
%token GLOBAL EXTERN SECTION WORD SKIP END LD ST CSRRD 
%token CSRWR PLUS MINUS LPARREN RPARREN SEMI COMMA ENDL

%%

program : programList END

programList: programList programElem
           | programElem

programElem: extern
           | global
           | section
           | labelSection

symbolList: symbolList COMMA IDENT {Assembler::getIdent($3, true);}
          | IDENT {Assembler::getIdent($1, true);}

extern: EXTERN symbolList

global: GLOBAL symbolList

section: SECTION IDENT

labelSection: LABEL instructionList

instructionList: instructionList instruction
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

operand: OPR_DEC 
       | OPR_HEX
       | OPR_STRING 
       | IDENT 
       | parrens

parrens: LPARREN parrensBodyList RPARREN

parrensBodyList: parrensBodyList plusMinus parrensBody
               | parrensBody

parrensBody: GPR_REG 
           | HEX

plusMinus: PLUS 
         | MINUS

symbolLiteralList: symbolLiteralList COMMA literal
                 | literal

literal: DEC 
       | HEX 
       | IDENT

%%
