#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array> 

#include "parser.tab.h"
// #include "lex.yy.c"
#include "../inc/assembler.hpp"

using namespace std;

extern int yyparse();
extern FILE *yyin, *yyout;

bool Assembler::secondPass;
map<string, RealocationEntry> Assembler::relocations;
map<string, Symbol> Assembler::symbols;
map<string, PoolOfLiterals> Assembler::pools;
map<string, Section> Assembler::sections;
string Assembler::currentSectionName;
int Assembler::instructionNum;
int Assembler::currentSectionSize;
string Assembler::currentDirective;
int Assembler::symSerialNum;
int Assembler::secSerialNum;

void Assembler::init(){
  secondPass = false;
  relocations.clear();
  symbols.clear();
  pools.clear();
  sections.clear();
  currentSectionName = "";
  currentSectionSize = 0;
  currentDirective = "";
  symSerialNum = 0;
  secSerialNum = 0;
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
  if(!secondPass && currentDirective.compare("extern") == 0){ // on je extern i prvi je prolaz
    
    cout << "Extern: " << name << endl;
    if (inTable(name)) {
        cout << "ERROR:Label already in table " << name << endl;
        exit(1);
    }

    Symbol s;
    s.name = name;
    s.section = "UND";
    s.offset = -1;
    s.isLocal = !isGlobal;
    s.serialNum = symSerialNum++;
    
    symbols.insert(make_pair(name, s));
    //sledece ispisi sve ove simbole negde
    //tabelaSimbola.push_back(new Simbol(s, "und", -1, "global"));
  }
}

void Assembler::startSection(string name){

  if(currentSectionName != ""){
    Section sec;
    sec.name = currentSectionName;
    sec.serialNum = secSerialNum++;
    sec.size = currentSectionSize;

    sections.insert(make_pair(currentSectionName, sec));

    if (inTable(currentSectionName)) {
        cout << "ERROR:Section already somewhere else! " << currentSectionName << endl;
        exit(1);
    }

    Symbol s;
    s.name = currentSectionName;
    s.section = "UND";
    s.offset = currentSectionSize;
    s.isLocal = true;
    s.serialNum = symSerialNum++;

    symbols.insert(make_pair(currentSectionName, s));
  }

  currentSectionName = name;
  currentSectionSize = 0;
  if(!secondPass){
    cout << name << endl;
  }
}

void Assembler::programEnd(){

  if(currentSectionName != ""){
    Section sec;
    sec.name = currentSectionName;
    sec.serialNum = secSerialNum++;
    sec.size = currentSectionSize;

    sections.insert(make_pair(currentSectionName, sec));
  }

}

void Assembler::directiveStart(string name){
  currentDirective = name;
  cout << name << endl;
}

void Assembler::directiveEnd(){
  currentDirective = "";
}

//promeniti deo za labele
void Assembler::labelStart(string name){

  name.erase(name.size()-1);

  if(currentSectionName == ""){
      cout << "ERROR: Label " << name << " must be defined inside of section!" << endl;
      exit(1);
  }
  map<string,Symbol>::iterator it = symbols.find(name);
  if(it!=symbols.end()){
      // if(entry->second.isDefined){
      //     cout<<"Label "<< name <<" defined on line "<<lineNumber<<" is aldready defined"<<endl;
      //     exit(1);
      // }
      if(!it->second.isLocal){
          cout<<"Label "<< name <<" is extern/not local"<<endl;
          exit(1);
      }
      if(it->second.isSection){
          cout << "Label "<< name << " is a section" << endl;
          exit(1);
      }
      // entry->second.isDefined=true;
      it->second.value = currentSectionSize;
      it->second.section=currentSectionName;
  }
  else{
      Symbol s;
      s.name = name;
      s.serialNum = symSerialNum++;
      s.section = currentSectionName;
      s.value = currentSectionSize;
      s.isLocal = true;
      s.isSection = false;
      // s.isDefined = true;
      // s.isExtern = false;

      symbols[name] = s;
  }

  cout << name << endl;
}

void Assembler::instructionPass(string name){

  //proveriti za word i skip - mozda se razlikuju malo za currentSectionSize

  currentSectionSize += 4;
  cout << name << endl;
}

void Assembler::getLiteral(string name){

}

//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////

bool Assembler::inTable(string name){
  
  auto it = symbols.find(name);

  if (it != symbols.end())
      return true;

  return false; 
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ispis svih elemenata mape
// for (const auto& pair : symbols) {
//   cout << pair.first << endl;
// }


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