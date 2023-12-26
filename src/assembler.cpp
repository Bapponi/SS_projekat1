#include <iostream>
#include "parser.tab.h"

extern int yyparse();
int main(){

  printf("ZIV SAM");
  yyparse();

  return 1;
}