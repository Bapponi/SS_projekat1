#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
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
map<string, vector<PoolOfLiterals>> Assembler::pools;
map<string, Section> Assembler::sections;
vector<PoolOfLiterals> Assembler::poolVector;
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
  poolVector.clear();
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

    pools.insert(make_pair(currentSectionName, poolVector));
    poolVector.clear();

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

    pools.insert(make_pair(currentSectionName, poolVector));
    poolVector.clear();

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
  fileOffset = 0;
}

void Assembler::directiveStart(string name){
  currentDirective = name;
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
          cout<<"ERROR: Label "<< name <<" is extern/not local"<<endl;
          exit(1);
      }
      if(it->second.isSection){
          cout << "ERROR: Label "<< name << " is a section" << endl;
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
}

void Assembler::instructionPass(string name){
  currentInstruction = name;
  currentSectionSize += 4;
  fileOffset += 4;
}

void Assembler::getLiteral(string name, string type){

  PoolOfLiterals p;

  if(type.compare("dec") == 0){
    int num = stoi(name);
    if(num > 2047 || num < -2048){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "dec"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      poolVector.push_back(p);
      hasPool = true;
    }

  }else if(type.compare("hex") == 0){
    name.erase(0, 2);
    long long num = stoll(name, nullptr, 16);

    if(num > 4095){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "hex"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      poolVector.push_back(p);
      hasPool = true;
    }

  }else{
    p.isSymbol = true;
    p.symbolAddress = currentSectionSize;
    p.symbolName = name;
    p.symbolValue = 0;

    poolVector.push_back(p);
    hasPool = true;
  }
}

void Assembler::getOperand(string name, string type){
  PoolOfLiterals p;
  if(type.compare("opr_dec") == 0){
    name.erase(0, 1);
    int num = stoi(name);
    if(num > 2047 || num < -2048){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "opr_dec"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      poolVector.push_back(p);
      hasPool = true;
    }

  }else if(type.compare("opr_hex") == 0){
    name.erase(0, 3);
    long long num = stoll(name, nullptr, 16);

    if(num > 4095){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "opr_hex"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      poolVector.push_back(p);
      hasPool = true;
    }

  }else if(type.compare("opr_string") == 0){
    name.erase(0, 1);
    p.isSymbol = true;
    p.symbolAddress = currentSectionSize;
    p.symbolName = name;
    p.symbolValue = 0;

    poolVector.push_back(p);
    hasPool = true;

  }else{
    p.isSymbol = true;
    p.symbolAddress = currentSectionSize;
    p.symbolName = name;
    p.symbolValue = 0;

    poolVector.push_back(p);
    hasPool = true;
  }
}

void Assembler::getParrensBody(string name, string type){
  PoolOfLiterals p;

  if(type.compare("hex") == 0){

    name.erase(0, 2);
    long long num = stoll(name, nullptr, 16);

    if(num > 4095){
      p.isSymbol = false;
      p.symbolAddress = currentSectionSize;
      p.symbolName = "hex"; //ne znam sta za ime, da li samo redni broj
      p.symbolValue = num;

      poolVector.push_back(p);
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

void Assembler::startSection2(string name){
  currentSectionName = name;
  currentSectionSize = 0;
}

void Assembler::programEnd2(){
  currentSectionName = "";
  currentSectionSize = 0;
  fileOffset = 0;
}

void Assembler::instructionPass2(string name, string op1, string op2){
  
  auto sec = sections.find(currentSectionName);

  if (sec == sections.end()) {
      cout << "\nKey: " << currentSectionName << " not found" << endl;
      exit(1);
  }

  if(op1 == "%sp") op1 = "%r14";
  if(op1 == "%pc") op1 = "%r15";
  if(op2 == "%sp") op2 = "%r14";
  if(op2 == "%pc") op2 = "%r15";

  sec->second.offsets.push_back(currentSectionSize);

  if(name.compare("halt") == 0){

    sec->second.data.push_back("00000000000000000000000000000000");

  }else if(name.compare("int") == 0){

    //generise softverski prekid
    sec->second.data.push_back("00010000000000000000000000000000");

  }else if(name.compare("iret") == 0){

    sec->second.data.push_back("10010001111011100000000000001000");
    sec->second.offsets.push_back(-1);
    sec->second.data.push_back("10010110000011100000111111111100");
    sec->second.offsets.push_back(-1);
    sec->second.data.push_back("10010001111011100000000000001000");

    currentSectionSize += 8;

  }else if(name.compare("ret ") == 0){

    sec->second.data.push_back("10010011111111100000000000000100");

  }else if(name.compare("push ") == 0){

    string code = "1000000111100000";

    op1 = op1.substr(2);
    code += getBits(op1, 4);

    code += "111111111100";
    sec->second.data.push_back(code);

  }else if(name.compare("pop ") == 0){

    string code = "10010011";

    op1 = op1.substr(2);
    code += getBits(op1, 4);

    code += "11100000000000000100";
    sec->second.data.push_back(code);

  }else if(name.compare("not ") == 0){
    string code = "01100000";

    op1 = op1.substr(2);
    code += getBits(op1, 4);
    code += getBits(op1, 4);

    code += "0000000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("xchg ") == 0){
    string code = "010000000000";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("add ") == 0){
    string code = "01010000";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("sub ") == 0){
    string code = "01010001";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("mul ") == 0){
    string code = "01010010";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("div ") == 0){
    string code = "01010011";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("and ") == 0){
    string code = "01100001";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec-> second.data.push_back(code);

  }else if(name.compare("or ") == 0){
    string code = "01100010";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("xor ") == 0){
    string code = "01100011";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("shl ") == 0){
    string code = "01110000";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("shr ") == 0){
    string code = "01110001";

    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op2, 4);
    code += getBits(op2, 4);
    code += getBits(op1, 4);

    code += "000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("beq ") == 0){
    string code = "0011";
    sec->second.data.push_back(code);

  }else if(name.compare("bne ") == 0){
    string code = "0011";
    sec->second.data.push_back(code);

  }else if(name.compare("bgt ") == 0){
    string code = "0011";
    sec->second.data.push_back(code);

  }else if(name.compare("jmp ") == 0){
    cout << "JMP" << endl;
    string code = "0011";
    sec->second.data.push_back(code);

  }else if(name.compare("call ") == 0){
    string code = "0010";

    //nesto

    code += "00000000";

    //nesto

    sec->second.data.push_back(code);

  }else if(name.compare("ld ") == 0){
    string code = "0";
    sec->second.data.push_back(code);

  }else if(name.compare("st ") == 0){
    string code = "1";
    sec->second.data.push_back(code);

  }else if(name.compare("csrrd ") == 0){
    string code = "10010000";

    op2 = op2.substr(2);
    code += getBits(op2, 4);
    
    if(op1 == "%status"){
      code += "0000";
    }else if(op1 == "%handler"){
      code += "0001";
    }else if(op1 == "%cause"){
      code += "0010";
    }else{
      cout << " ERROR: invalid CSR register: " << op1 << endl;
    }

    code += "0000000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("csrwr ") == 0){
    string code = "10010100";

    op1 = op1.substr(2);

    if(op2 == "%status"){
      code += "0000";
    }else if(op2 == "%handler"){
      code += "0001";
    }else if(op2 == "%cause"){
      code += "0010";
    }else{
      cout << " ERROR: invalid CSR register: " << op2 << endl;
    }

    code += getBits(op1, 4);

    code += "0000000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare(".skip ") == 0){
    string code = "2";
    sec->second.data.push_back(code);

  }else if(name == ".word "){ //ovde je problem da kaze da ima problem sa string-om .word_
    string code = "3";
    sec->second.data.push_back(code);
  }else{
    cout << "Netacna operacija!!!" << endl;
  }

  currentSectionSize += 4;
  fileOffset += 4;
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

string Assembler::getBits(const string& stringInt, int nBits) {
    // Convert string to integer
    int intValue = stoi(stringInt);

    // Ensure the integer value can be represented in nBits
    if (intValue >= (1 << nBits)) {
        cout << "Error: Integer value exceeds " << nBits << " bits representation." << endl;
        return "-1";
    }

    // Convert integer to binary representation
    bitset<32> bits(intValue);
    string bitString = bits.to_string();

    // Extract the last nBits characters
    return bitString.substr(32 - nBits);
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
       << setw(10) << "HasPool" << setw(15) << "PoolSize" << endl;

  for (const auto& entry : sections) {
      const Section& section = entry.second;

      cout << setw(15) << section.name << setw(10) << section.serialNum
                << setw(10) << section.size << setw(10) << section.hasPool
                << setw(15) << section.poolSize << endl;
  }

  cout << "\n" << endl;

  cout << setw(15) << "Section" << setw(20) << "Offsets" << setw(36) << "Data" << endl;

  for (const auto& entry : symbolMap) {
      const vector<long long>& offsetsVector = entry.second.offsets;
      const vector<string>& dataVector = entry.second.data;

      for (int i = 0; i < offsetsVector.size(); i++) {
          cout << setw(15) << entry.first << setw(20) << offsetsVector.at(i) << setw(36) << dataVector.at(i) << endl;
      }
  }

  cout << "\n" << endl;
}

void Assembler::displayPoolTable(const map<string, vector<PoolOfLiterals>>& symbolMap){
  
  cout << "    ----------------------------POOLS---------------------------" << endl;
  cout << setw(15) << "Key" << setw(15) << "SymbolName" << setw(15) << "SymbolAddress" 
       << setw(20) << "SymbolValue" << setw(15) << "IsSymbol" << endl;

  for (const auto& entry : symbolMap) {
      const vector<PoolOfLiterals>& literalsVector = entry.second;

      for (const auto& literal : literalsVector) {
          cout << setw(15) << entry.first << setw(15) << literal.symbolName << setw(15) << literal.symbolAddress
               << setw(20) << literal.symbolValue << setw(15) << (literal.isSymbol ? "true" : "false")
               << endl;
      }
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