#include <iostream>
#include <fstream>
#include <string>

#include "parser.tab.h"

using namespace std;

extern int yyparse();
extern FILE *yyin, *yyout;
extern char yytext[];
extern char * yyget_text();
// extern char *token;

int main(int argc, char* argv[]){

  FILE *file = fopen("./test/nivo-a/main.s", "r");

  if ((!file)) {
    printf("I can't open the file!\n");
    return -1;
  }
  
  // set lex to read from it instead of defaulting to STDIN:
  yyin = file;
  
  // lex through the input:
  while(yyparse()){
    char * text = yyget_text();
  }
  fclose(file);

  printf("Prosao ceo fajl bez greske");
  yyparse();

  return 1;
}