#include <iostream>
#include <fstream>
#include <string>

#include "parser.tab.h"
#include "../inc/assembler.hpp"

using namespace std;

extern int yyparse();
extern FILE *yyin, *yyout;

bool Assembler::secondPass;

void Assembler::init(){
  printf("Test\n");
  secondPass = false;
}

void Assembler::passFile(){
  FILE *file = fopen("./test/nivo-a/main.s", "r");

  if ((!file)) {
    printf("I can't open the file!\n");
    return;
  }
  
  yyin = file;

  while(yyparse());

  string boolString = to_string(secondPass);
  cout << "SecondPass: " << boolString << endl;

  if(secondPass == false){
    Assembler::init();
    secondPass = true;
  }
  else
    secondPass = false;

  fclose(file);

}

int main(int argc, char* argv[]){

  Assembler::passFile();
  Assembler::passFile();

  printf("Prosao ceo fajl bez greske\n");

  return 1;
}