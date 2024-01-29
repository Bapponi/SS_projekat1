#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array> 
#include <iomanip> //za setw

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
string Assembler::currentInstruction;
int Assembler::fileOffset;
bool Assembler::hasPool;

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
  currentInstruction = "";
  fileOffset = 0;
  hasPool = false;
}

void Assembler::passFile(string fileName, int fileNum, int passNum){
  const char * filePath = fileName.c_str();
  FILE *file = fopen(filePath, "r");

  if ((!file)) {
    printf("I can't open the file!\n");
    return;
  }
  
  yyin = file;

  if(passNum == 1 && fileNum == 0 && !secondPass)
    Assembler::init();

  if(passNum == 2)
    secondPass = true;

  while(yyparse());

  string boolString = to_string(secondPass);
  cout << "SecondPass: " << boolString << endl;

  displaySymbolTable(symbols);
  displaySectionTable(sections);
  displayPoolTable(pools);
  displayRelocationTable(relocations);

  fclose(file);

}

//global ne obradjujemo u prvom prolazu, ali zato to radimo sa extern-om
void Assembler::getIdent(string name, bool isGlobal){
  if(currentDirective.compare("extern") == 0){ // on je extern i prvi je prolaz
    
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
    s.isSection = false;
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
    sec.hasPool = hasPool;
    sec.poolSize = 0;

    sections.insert(make_pair(currentSectionName, sec));

    hasPool = false;

    if (inTable(currentSectionName)) {
        cout << "ERROR:Section already somewhere else! " << currentSectionName << endl;
        exit(1);
    }

    Symbol s;
    s.name = currentSectionName;
    s.section = "UND";
    s.offset = fileOffset - currentSectionSize;
    s.isLocal = true;
    s.isSection = true;
    s.serialNum = symSerialNum++;
    s.value = 0;

    symbols.insert(make_pair(currentSectionName, s));
  }

  currentSectionName = name;
  currentSectionSize = 0;
    
  cout << name << endl;
}

void Assembler::programEnd(){

  if(currentSectionName != ""){
    Section sec;
    sec.name = currentSectionName;
    sec.serialNum = secSerialNum++;
    sec.size = currentSectionSize;
    sec.hasPool = hasPool;
    sec.poolSize = 0;

    sections.insert(make_pair(currentSectionName, sec));

    hasPool = false;

    if (inTable(currentSectionName)) {
      cout << "ERROR:Section already somewhere else! " << currentSectionName << endl;
      exit(1);
    }

    Symbol s;
    s.name = currentSectionName;
    s.section = "UND";
    s.offset = fileOffset - currentSectionSize;
    s.isLocal = true;
    s.isSection = true;
    s.serialNum = symSerialNum++;
    s.value = 0;

    symbols.insert(make_pair(currentSectionName, s));
  }

  currentSectionName = "";
  currentSectionSize = 0;
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
      s.value = 0;
      s.offset = currentSectionSize;
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
  currentInstruction = name;
  currentSectionSize += 4;
  fileOffset += 4;
  cout << name << endl;
}

void Assembler::getLiteral(string name, string type){
  cout << " Literal: " << name << ", tip: " << type << endl;
  PoolOfLiterals p;

  if(type.compare("dec") == 0){
    cout << "DEC in!!!" << endl;
    int num = stoi(name);
    if(num > 2047 || num < -2048){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "dec"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      pools[name] = p;
      hasPool = true;
    }

  }else if(type.compare("hex") == 0){
    cout << "HEX in!!!" << endl;
    name.erase(0, 2);
    long long num = stoll(name, nullptr, 16);

    if(num > 4095){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "hex"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      pools[name] = p;
      hasPool = true;
    }

  }else{
    cout << "Ident in!!!" << endl;
    p.isSymbol = true;
    p.symbolAddress = currentSectionSize;
    p.symbolName = name;
    p.symbolValue = 0;

    pools[name] = p;
    hasPool = true;
  }
}

void Assembler::getOperand(string name, string type){
  cout << " Operand: " << name << ", tip: " << type << endl;
  PoolOfLiterals p;
  if(type.compare("opr_dec") == 0){
    cout << "OPR_DEC in!!!" << endl;
    name.erase(0, 1);
    int num = stoi(name);
    if(num > 2047 || num < -2048){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "opr_dec"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      pools[name] = p;
      hasPool = true;
    }

  }else if(type.compare("opr_hex") == 0){
    cout << "OPR_HEX in!!!" << endl;
    name.erase(0, 3);
    long long num = stoll(name, nullptr, 16);

    if(num > 4095){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "opr_hex"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      pools[name] = p;
      hasPool = true;
    }

  }else if(type.compare("opr_string") == 0){
    cout << "OPR_STRING in!!!" << endl;
    name.erase(0, 1);
    p.isSymbol = true;
    p.symbolAddress = currentSectionSize;
    p.symbolName = name;
    p.symbolValue = 0;

    pools[name] = p;
    hasPool = true;

  }else{
    cout << "Ident in!!!" << endl;
    p.isSymbol = true;
    p.symbolAddress = currentSectionSize;
    p.symbolName = name;
    p.symbolValue = 0;

    pools[name] = p;
    hasPool = true;
  }
}

void Assembler::getParrensBody(string name, string type){
  cout << " ParrensBody: " << ", tip: " << type << endl;
  PoolOfLiterals p;

  if(type.compare("hex") == 0){

    cout << "HEX in parrens in!!!" << endl;
    name.erase(0, 2);
    long long num = stoll(name, nullptr, 16);

    if(num > 4095){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "hex"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      pools[name] = p;
      hasPool = true;
    }

  }else{} //deo sa registrom 

}
//////////////////DRUGI PROLAZ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////DRUGI PROLAZ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////DRUGI PROLAZ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////DRUGI PROLAZ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////DRUGI PROLAZ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////DRUGI PROLAZ////////////////////////////////////////////////////////////////////////////////////////////

// void Assembler::startSection2(string name){

// }

void Assembler::instructionPass2(){

}

//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////
//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////
//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////
//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////
//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////
//////////////////POMOCNE FUNKCIJE////////////////////////////////////////////////////////////////////////////////////////

bool Assembler::inTable(string name){
  
  auto it = symbols.find(name);

  if (it != symbols.end())
      return true;

  return false; 
}

void Assembler::displaySymbolTable(const map<string, Symbol>& symbolMap){
  cout << "       -------------------------------SYMBOLS-----------------------------------" << endl;
  cout << setw(15) << "Name" << setw(10) << "SerialNum" << setw(10) << "Value"
       << setw(10) << "IsLocal" << setw(15) << "Section" << setw(10) << "IsSection"
       << setw(10) << "Offset" << endl;
  
  for (const auto& entry : symbolMap) {
      const Symbol& symbol = entry.second;
      cout << setw(15) << symbol.name << setw(10) << symbol.serialNum
           << setw(10) << symbol.value << setw(10) << symbol.isLocal
           << setw(15) << symbol.section << setw(10) << symbol.isSection
           << setw(10) << symbol.offset << endl;
  }

  cout << "\n" << endl;
}

void Assembler::displaySectionTable(const map<string, Section>& symbolMap){
  cout << "       ------------------------------------------SECTIONS-------------------------------------------" << endl;
  cout << setw(15) << "Name" << setw(10) << "SerialNum" << setw(10) << "Size"
       << setw(10) << "HasPool" << setw(15) << "PoolSize" << setw(20) << "Offsets"
       << setw(20) << "Data" << endl;

  for (const auto& entry : sections) {
      const Section& section = entry.second;

      string offsetsStr;
      for (const auto& offset : section.offsets) {
          offsetsStr += to_string(offset) + " ";
      }

      string dataStr(section.data.begin(), section.data.end());

      cout << setw(15) << section.name << setw(10) << section.serialNum
                << setw(10) << section.size << setw(10) << section.hasPool
                << setw(15) << section.poolSize << setw(20) << offsetsStr
                << setw(20) << dataStr << endl;
  }

  cout << "\n" << endl;
}

void Assembler::displayPoolTable(const map<string, PoolOfLiterals>& symbolMap){
  cout << "       -------------------------POOLS-----------------------" << endl;
  cout << setw(15) << "Name" << setw(15) << "Address" << setw(15) << "Value"
       << setw(15) << "IsSymbol" << endl;

  for (const auto& entry : pools) {
      const PoolOfLiterals& pool = entry.second;

      cout << setw(15) << pool.symbolName << setw(15) << pool.symbolAddress
           << setw(15) << pool.symbolValue << setw(15) << pool.isSymbol << endl;
  }

  cout << "\n" << endl;
}

void Assembler::displayRelocationTable(const map<string, RealocationEntry>& symbolMap){
  
  cout << "       -------------------------RELOCATIONS-----------------------" << endl;
  cout << setw(15) << "Section" << setw(15) << "Offset" << setw(15) << "Symbol"
       << setw(15) << "Addent" << endl;

  for (const auto& entry : relocations) {
      const RealocationEntry& relocations = entry.second;

      // Print table row
      cout << setw(15) << relocations.section << setw(15) << relocations.offset
           << setw(15) << relocations.symbol << setw(15) << relocations.addent << endl;
  }

  cout << "\n" << endl;
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
  for(int i = 0; i < inputFiles.size(); i++){
    Assembler::passFile(srcFolder + inputFiles[i], i, 2);
  }

  printf("Prosao ceo fajl bez greske\n");

  return 1;
}