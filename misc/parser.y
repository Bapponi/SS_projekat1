%{
#include "../inc/assembler.hpp"
#include <stdio.h>

extern int yyparse(void);
extern int yyerror(const char *s);

extern int yylex();   // Bison needs to know how to call the lexer
extern char* yytext;  // Bison needs to know about yytext
%}

%union {
    int num;
    char* string;
}

%token <string> ONE_WORD_INST TWO_WORD_INST
%token <string> THREE_WORD_INST FOUR_WORD_INST CALL_JUMP
%token <string> STRING OPR_STRING IDENT LABEL CSR_REG GPR_REG
%token <string> GLOBAL EXTERN SECTION WORD SKIP END LD ST CSRRD 
%token <string> CSRWR PLUS MINUS LPARREN RPARREN SEMI COMMA ENDL
%token <string> OPR_HEX HEX OPR_DEC DEC

%%

program : programList end

end: END {Assembler::programEnd();}

programList: programList programElem
           | programElem

programElem: extern
           | global
           | section
           | labelSection

symbolList: symbolList COMMA IDENT {Assembler::getIdent($3, true);}
          | IDENT {Assembler::getIdent($1, true);}

extern: externStart symbolList {Assembler::directiveEnd();}

externStart: EXTERN {Assembler::directiveStart("extern");}

global: globalStart symbolList {Assembler::directiveEnd();}

globalStart: GLOBAL {Assembler::directiveStart("global");}

section: SECTION IDENT {Assembler::startSection($2);}

labelSection: labelStart instructionList

labelStart: LABEL {Assembler::labelStart($1);}

instructionList: instructionList instruction
               | /* epsilon */

instruction: ONE_WORD_INST                                      {Assembler::instructionPass($1, "", "");}
           | TWO_WORD_INST GPR_REG                              {Assembler::instructionPass($1, $2, "");}
           | THREE_WORD_INST GPR_REG COMMA GPR_REG              {Assembler::instructionPass($1, $2, $4);}
           | FOUR_WORD_INST GPR_REG COMMA GPR_REG COMMA operand {Assembler::instructionPass($1, $2, $4);}
           | CALL_JUMP literal                                  {Assembler::instructionPass($1, "", "");}
           | ld LD operand COMMA GPR_REG                        {Assembler::instructionPass($2, $5, "");}
           | st ST GPR_REG COMMA operand                        {Assembler::instructionPass($2, $3, "");}
           | CSRRD CSR_REG COMMA GPR_REG                        {Assembler::instructionPass($1, $2, $4);}
           | CSRWR GPR_REG COMMA CSR_REG                        {Assembler::instructionPass($1, $2, $4);}
           | word WORD symbolLiteralList                        {Assembler::instructionPass($2, "", "");}
           | skip SKIP literal                                  {Assembler::instructionPass($2, "", "");}

ld: {Assembler::instructionName("ld ");}

st: {Assembler::instructionName("st ");}

word: {Assembler::instructionName(".word ");}

skip: {Assembler::instructionName(".skip ");}

operand: OPR_DEC    {Assembler::getOperand($1, "opr_dec");}
       | OPR_HEX    {Assembler::getOperand($1, "opr_hex");}
       | OPR_STRING {Assembler::getOperand($1, "opr_string");}
       | IDENT      {Assembler::getOperand($1, "ident");}
       | parrens

parrens: LPARREN parrensBodyList RPARREN

parrensBodyList: parrensBodyList plusMinus parrensBody
               | parrensBody

parrensBody: GPR_REG    {Assembler::getParrensBody($1, "gpr_reg");}
           | HEX        {Assembler::getParrensBody($1, "hex");}

plusMinus: PLUS 
         | MINUS

symbolLiteralList: symbolLiteralList COMMA literal
                 | literal

literal: DEC    {Assembler::getLiteral($1, "dec");}
       | HEX    {Assembler::getLiteral($1, "hex");}
       | IDENT  {Assembler::getLiteral($1, "ident");}

%%
