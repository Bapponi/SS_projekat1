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
map<string, RealocationEntry> Assembler::relocations;
map<string, Symbol> Assembler::symbols;
map<string, PoolOfLiterals> Assembler::pools;
map<string, Section> Assembler::sections;
string Assembler::currentSection;
int Assembler::instructionNum;
int Assembler::currentSectionSize;
string Assembler::currentDirective;

void Assembler::init(){
  secondPass = false;
  relocations.clear();
  symbols.clear();
  pools.clear();
  sections.clear();
  currentSection = "";
  currentSectionSize = 0;
  currentDirective = "";
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

//global ne obradjujemo u prvom prolazu, ali zato to radimo sa extern-om
void Assembler::getIdent(string name, bool isGlobal){
  if(!secondPass){
    cout << name << endl;
  }
}

void Assembler::startSection(string name){
  //ovde smestiti staru sekciju
  currentSection = name;
  currentSectionSize = 0;
  if(!secondPass){
    cout << name << endl;
  }
}

void Assembler::directiveStart(string name){
  currentDirective = name;
  cout << name << endl;
}

void Assembler::directiveEnd(){
  currentDirective = "";
}

void Assembler::labelStart(string name){
  cout << name << endl;
}

void Assembler::instructionPass(string name){
  currentSectionSize += 4;
  cout << name << endl;
}

void Assembler::getLiteral(string name){

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