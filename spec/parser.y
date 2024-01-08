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

program: extrGlobList sectionList END

extrGlobList: extrGlob extrGlobList
            | /* epsilon */

extrGlob: extern | global

extern: EXTERN symbolList{
    Assembler::test();
}

global: GLOBAL symbolList

symbolList: IDENT COMMA symbolList
          | IDENT

sectionList: section sectionList
            | /* epsilon */

section: SECTION IDENT sectionPartList

sectionPartList: sectionPart sectionPartList
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

operand: OPR_DEC | OPR_HEX | OPR_STRING | IDENT | parrens

parrens: LPARREN parrensBodyList RPARREN

parrensBodyList: parrensBody plusMinus parrensBodyList
               | parrensBody

parrensBody: GPR_REG | HEX

plusMinus: PLUS | MINUS

symbolLiteralList: literal COMMA symbolLiteralList
                 | literal

literal: DEC | HEX | IDENT

%%
