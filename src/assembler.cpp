#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array> 

#include "parser.tab.h"
#include "../inc/assembler.hpp"

using namespace std;

extern int yyparse();
extern FILE *yyin, *yyout;

bool Assembler::secondPass;

void Assembler::init(){
  secondPass = false;
  // inputFiles.clear();
  // relocations.clear();
  // symbols.clear();
  // poolOfLiterals.clear();
  // sections.clear();
}

void Assembler::getIdent(string name, bool isGlobal){
  cout << name << endl;
}

void Assembler::passFile(string fileName, int fileNum, int passNum){
  
  const char * filePath = fileName.c_str();
  FILE *file = fopen(filePath, "r");

  if ((!file)) {
    printf("I can't open the file!\n");
    return;
  }
  
  yyin = file;

  if(passNum == 1 && fileNum == 0)
    Assembler::init();

  if(passNum == 2)
    secondPass = true;

  while(yyparse());

  string boolString = to_string(secondPass);
  cout << "SecondPass: " << boolString << endl;

  fclose(file);

}

array<string,1> inputFiles{ 
  // "handler.s", 
  // "isr_software.s", 
  // "isr_terminal.s",
  // "isr_timer.s",
  "main.s",
  // "math.s"
};

string srcFolder = "./test/nivo-a/";

int main(int argc, char* argv[]){

  //prvi prolaz
  for(int i = 0; i < inputFiles.size(); i++){
    Assembler::passFile(srcFolder + inputFiles[i], i, 1);
  }

  //drugi prolaz
  // for(int i = 0; i < inputFiles.size(); i++){
  //   Assembler::passFile(srcFolder + inputFiles[i], i, 2);
  // }

  printf("Prosao ceo fajl bez greske\n");

  return 1;
}