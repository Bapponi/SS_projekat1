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

end: END {if(!Assembler::secondPass) Assembler::programEnd(); else Assembler::programEnd2();}

programList: programList programElem
           | programElem

programElem: extern
           | global
           | section
           | labelSection

symbolList: symbolList COMMA IDENT {if(!Assembler::secondPass) Assembler::getIdent($3, true);}
          | IDENT {if(!Assembler::secondPass) Assembler::getIdent($1, true);}

extern: externStart symbolList {if(!Assembler::secondPass) Assembler::directiveEnd();}

externStart: EXTERN {if(!Assembler::secondPass) Assembler::directiveStart("extern");}

global: globalStart symbolList {if(!Assembler::secondPass) Assembler::directiveEnd();}

globalStart: GLOBAL {if(!Assembler::secondPass) Assembler::directiveStart("global");}

section: SECTION IDENT {if(!Assembler::secondPass) Assembler::startSection($2); else Assembler::startSection2($2);}

labelSection: labelStart instructionList

labelStart: LABEL {if(!Assembler::secondPass) Assembler::labelStart($1);}

instructionList: instructionList instruction
               | /* epsilon */

instruction: ONE_WORD_INST                                      {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, "", "");
                                                                }
           | TWO_WORD_INST GPR_REG                              {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, $2, "");
                                                                }
           | THREE_WORD_INST GPR_REG COMMA GPR_REG              {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, $2, $4);
                                                                }
           | FOUR_WORD_INST GPR_REG COMMA GPR_REG COMMA operand {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, $2, $4);
                                                                }
           | CALL_JUMP literal                                  {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, "", "");
                                                                }
           | ld LD operand COMMA GPR_REG                           {if(!Assembler::secondPass) Assembler::instructionPass($2); 
                                                                      else Assembler::instructionPass2($2, $5, "");
                                                                }
           | st ST GPR_REG COMMA operand                           {if(!Assembler::secondPass) Assembler::instructionPass($2); 
                                                                      else Assembler::instructionPass2($2, $3, "");
                                                                }
           | CSRRD CSR_REG COMMA GPR_REG                        {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, $2, $4);
                                                                }
           | CSRWR GPR_REG COMMA CSR_REG                        {if(!Assembler::secondPass) Assembler::instructionPass($1); 
                                                                      else Assembler::instructionPass2($1, $2, $4);
                                                                }
           | word WORD symbolLiteralList                             {if(!Assembler::secondPass) Assembler::instructionPass($2); 
                                                                      else Assembler::instructionPass2($2, "", "");
                                                                }
           | skip SKIP literal                                       {if(!Assembler::secondPass) Assembler::instructionPass($2); 
                                                                      else Assembler::instructionPass2($2, "", "");
                                                                }

ld: {Assembler::instructionName("ld ");}

st: {Assembler::instructionName("st ");}

word: {Assembler::instructionName(".word ");}

skip: {Assembler::instructionName(".skip ");}

operand: OPR_DEC    {if(!Assembler::secondPass) Assembler::getOperand($1, "opr_dec"); 
                     else Assembler::getOperand2($1, "opr_dec");
                    }
       | OPR_HEX    {if(!Assembler::secondPass) Assembler::getOperand($1, "opr_hex"); 
                     else Assembler::getOperand2($1, "opr_hex");
                    }
       | OPR_STRING {if(!Assembler::secondPass) Assembler::getOperand($1, "opr_string"); 
                     else Assembler::getOperand2($1, "opr_string");
                    }
       | IDENT      {if(!Assembler::secondPass) Assembler::getOperand($1, "ident"); 
                     else Assembler::getOperand2($1, "ident");
                    }
       | parrens

parrens: LPARREN parrensBodyList RPARREN

parrensBodyList: parrensBodyList plusMinus parrensBody
               | parrensBody

parrensBody: GPR_REG    {if(!Assembler::secondPass) Assembler::getParrensBody($1, "gpr_reg"); 
                            else Assembler::getParrensBody2($1, "gpr_reg");
                        }
           | HEX        {if(!Assembler::secondPass) Assembler::getParrensBody($1, "hex");
                            else Assembler::getParrensBody2($1, "hex");
                        }

plusMinus: PLUS 
         | MINUS

symbolLiteralList: symbolLiteralList COMMA literal
                 | literal

literal: DEC    {if(!Assembler::secondPass) Assembler::getLiteral($1, "dec");
                     else Assembler::getLiteral2($1, "dec");
                }
       | HEX    {if(!Assembler::secondPass) Assembler::getLiteral($1, "hex");
                     Assembler::getLiteral2($1, "hex");
                }
       | IDENT  {if(!Assembler::secondPass) Assembler::getLiteral($1, "ident");
                     Assembler::getLiteral2($1, "ident");
                }

%%
