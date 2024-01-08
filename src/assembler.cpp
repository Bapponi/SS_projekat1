#include <iostream>
#include <fstream>
#include <string>

#include "parser.tab.h"
#include "../inc/assembler.hpp"

using namespace std;

extern int yyparse();
extern FILE *yyin, *yyout;
extern char yytext[];
extern char * yyget_text();

void Assembler::test(){
  printf("Test\n");
}

int main(int argc, char* argv[]){

  FILE *file = fopen("./test/nivo-a/main.s", "r");

  if ((!file)) {
    printf("I can't open the file!\n");
    return -1;
  }
  
  yyin = file;
  
  while(yyparse());
  fclose(file);

  printf("Prosao ceo fajl bez greske\n");

  return 1;
}