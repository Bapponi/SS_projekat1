#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
#include <iomanip>

#include "parser.tab.h"
#include "../inc/assembler.hpp"

using namespace std;

extern int yyparse();
extern FILE *yyin, *yyout;

map<string, vector<RealocationEntry>> Assembler::relocations;
map<string, Symbol> Assembler::symbols;
map<string, vector<PoolOfLiterals>> Assembler::pools;
map<string, Section> Assembler::sections;
vector<PoolOfLiterals> Assembler::poolVector;
map<string, OffsetAlter> Assembler::offsetAlters;
vector<int> Assembler::sectionSizes;
vector<int> Assembler::poolSizes;

string Assembler::fileOutput;
string Assembler::currentSectionName;
int Assembler::currentSectionSize;
string Assembler::currentDirective;
int Assembler::symSerialNum;
int Assembler::secSerialNum;
string Assembler::currentInstruction;
int Assembler::fileOffset;
bool Assembler::hasPool;
int Assembler::poolOffset;
long long Assembler::skipWordNum;

string Assembler::currentOperandOffset;
bool Assembler::hasPool2;
int Assembler::skipNum;
bool Assembler::inParrens;
string Assembler::parrensReg;
string Assembler::parrensHex;
bool Assembler::inOprString;

void Assembler::init(){
  relocations.clear();
  symbols.clear();
  pools.clear();
  sections.clear();
  poolVector.clear();
  offsetAlters.clear();
  sectionSizes.clear();
  poolSizes.clear();

  fileOutput = "";
  currentSectionName = "";
  currentSectionSize = 0;
  currentDirective = "";
  symSerialNum = 0;
  secSerialNum = 0;
  currentInstruction = "";
  fileOffset = 0;
  hasPool = false;
  poolOffset = 0;
  skipWordNum = -1;

  currentOperandOffset = " OFFSET ";
  hasPool2 = false;
  skipNum = -1;
  inParrens = false;
  parrensReg = "";
  parrensHex = "";
  inOprString = false;
}

void Assembler::passFile(string fileName, string fileOut){

  Assembler::init();
  fileOutput = fileOut;
  const char * filePath = fileName.c_str();
  FILE *file = fopen(filePath, "r");

  if ((!file)) {
    printf("Ne moze da se otvori fajl!\n");
    return;
  }
  
  yyin = file;

  while(yyparse());
    
  addPoolToSec();
  displaySymbolTable(symbols);
  displaySectionTable(sections);
  displayPoolTable(pools);
  displayRelocationTable(relocations);
  displayOffsetsTable(offsetAlters);
  makeOutputFile();

  fclose(file);

}

void Assembler::instructionName(string name){ //////////AAAAAAAAAAA
  currentInstruction = name;
}

void Assembler::directiveStart(string name){
  currentDirective = name;
}

void Assembler::directiveEnd(){
  currentDirective = "";
}

void Assembler::getIdent(string name, bool isGlobal){

  if (inTable(name)) {
    cout << "ERROR: Label already in table " << name << endl;
    exit(1);
  }

  Symbol s;
  s.name = name;
  s.offset = 0;
  s.isLocal = !isGlobal;
  s.isSection = false;
  s.serialNum = symSerialNum++;
  s.value = 0;

  if(currentDirective.compare("extern") == 0){
    s.section = "UND";
  }else{
    s.section = "GLOB";
  }
  
  symbols.insert(make_pair(name, s));

}

void Assembler::labelStart(string name){

  name.erase(name.size()-1);

  if(currentSectionName == ""){
    cout << "ERROR: Label " << name << " must be defined inside of section!" << endl;
    exit(1);
  }

  map<string,Symbol>::iterator itSym = symbols.find(name);
  if(itSym!=symbols.end()){

    if(!itSym->second.isLocal && itSym->second.section == "UND"){
      cout<<"ERROR: Label "<< name <<" is extern variable!!!"<<endl;
      exit(1);
    }
    if(itSym->second.isSection){
      cout << "ERROR: Label "<< name << " is a section!!!" << endl;
      exit(1);
    }

    itSym->second.value = currentSectionSize;
    itSym->second.section = currentSectionName;
  }
  else{
    Symbol s;
    s.name = name;
    s.serialNum = symSerialNum++;
    s.section = currentSectionName;
    s.value = currentSectionSize;
    s.offset = 0;
    s.isLocal = true;
    s.isSection = false;

    symbols[name] = s;
  }
}

void Assembler::startSection(string name){

  if(currentSectionName != ""){
    map<string, Section>::iterator itSec = sections.find(currentSectionName);
    itSec->second.size = currentSectionSize;

    pools.insert(make_pair(currentSectionName, poolVector));
    poolVector.clear();

    if (inTable(currentSectionName)) {
      cout << "ERROR: Section already somewhere else! " << currentSectionName << endl;
      exit(1);
    }
    hasPool = false;

    Symbol s;
    s.name = currentSectionName;
    s.section = "UND";
    s.offset = 0;
    s.isLocal = true;
    s.isSection = true;
    s.serialNum = symSerialNum++;
    s.value = 0;

    symbols.insert(make_pair(currentSectionName, s));

    OffsetAlter oa;
    oa.sectionSize = sectionSizes;
    oa.poolSize = poolSizes;
    
    offsetAlters.insert(make_pair(currentSectionName, oa));
    sectionSizes.clear();
    poolSizes.clear();

    map<string,vector<PoolOfLiterals>>::iterator itPool = pools.find(currentSectionName);
    for (int i = 0; i < itPool->second.size(); i++) {
      itPool->second[i].symbolAddress += currentSectionSize;
    }
  }

  Section sec;
  sec.name = name;
  sec.serialNum = secSerialNum++;
  sec.size = 0;
  sec.hasPool = false;
  sec.poolSize = 0;
  sec.offsets = vector<long long>();
  sec.data = vector<string>();
  
  sections.insert(make_pair(name, sec));

  currentSectionName = name;
  poolOffset = 0;
  currentSectionSize = 0;
}

void Assembler::programEnd(){

  map<string, Section>::iterator itSec = sections.find(currentSectionName);
  itSec->second.size = currentSectionSize;

  pools.insert(make_pair(currentSectionName, poolVector));
  poolVector.clear();

  if (inTable(currentSectionName)) {
    cout << "ERROR: Section already somewhere else! " << currentSectionName << endl;
    exit(1);
  }
  hasPool = false;

  Symbol s;
  s.name = currentSectionName;
  s.section = "UND";
  s.offset = 0;
  s.isLocal = true;
  s.isSection = true;
  s.serialNum = symSerialNum++;
  s.value = 0;

  symbols.insert(make_pair(currentSectionName, s));

  OffsetAlter oa;
  oa.sectionSize = sectionSizes;
  oa.poolSize = poolSizes;
  
  offsetAlters.insert(make_pair(currentSectionName, oa));
  sectionSizes.clear();
  poolSizes.clear();

  map<string,vector<PoolOfLiterals>>::iterator itPool = pools.find(currentSectionName);
  for (int i = 0; i < itPool->second.size(); i++) {
    itPool->second[i].symbolAddress += currentSectionSize;
  }

  currentSectionName = "";
  currentSectionSize = 0;
  fileOffset = 0;
  poolOffset = 0;

  for (auto itSec = symbols.begin(); itSec != symbols.end(); ++itSec) {
    string secName = itSec->first;
    vector<RealocationEntry> vre;
    relocations.insert(make_pair(secName, vre));
  }

  for (auto itPool = pools.begin(); itPool != pools.end(); ++itPool) {
    
    vector<PoolOfLiterals> v = itPool->second;

    for (size_t i = 0; i < v.size(); i++) {

      if(v[i].isSymbol == true){

        auto itSym = symbols.find(v[i].symbolName);
        Symbol s = itSym->second;
        RealocationEntry re;
        re.section = itPool->first;
        re.offset = v[i].symbolAddress;

        if(s.isLocal && !s.isSection && s.section != "UND"){
          re.symbol = itPool->first;
          re.addent = s.value;
        }else{
          re.symbol = s.name;
          re.addent = 0;
        }

        auto itRel = relocations.find(itPool->first);
        itRel->second.push_back(re);
      }

    }
  }

  for (auto& entry : sections) {
    vector<long long>& offsetsVector = entry.second.offsets;
    vector<string>& dataVector = entry.second.data;
    OffsetAlter oa = offsetAlters.find(entry.first)->second;
  
    int j = 0;
    for (int i = 0; i < offsetsVector.size(), j < oa.sectionSize.size() ; i++) {
      if(offsetsVector.at(i) == oa.sectionSize.at(j)){
        string a = to_string(oa.poolSize.at(j) + entry.second.size - oa.sectionSize.at(j) - 4);
        a = getBits(a, 12);
        dataVector.at(i) = dataVector.at(i).substr(0, dataVector.at(i).length() - 12) + a;
        j++;
      }
    }
  }
}

void Assembler::instructionPass(string name, string op1, string op2){

  auto sec = sections.find(currentSectionName);

  if (sec == sections.end()) {
    cout << "\nERROR: Key: " << currentSectionName << " was't found!!!" << endl;
    exit(1);
  }

  if(op1 == "%sp") op1 = "%r14";
  if(op1 == "%pc") op1 = "%r15";
  if(op2 == "%sp") op2 = "%r14";
  if(op2 == "%pc") op2 = "%r15";

  if(name != ".word "){
    sec->second.offsets.push_back(currentSectionSize);
  }

  if(name.compare("halt") == 0){

    sec->second.data.push_back("00000000000000000000000000000000");

  }else if(name.compare("int") == 0){

    sec->second.data.push_back("00010000000000000000000000000000");

  }else if(name.compare("iret") == 0){

    sec->second.data.push_back("11110000000000000000000000000000");

  }else if(name.compare("ret") == 0){

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
      exit(1);
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
      exit(1);
    }

    code += getBits(op1, 4);

    code += "0000000000000000";
    sec->second.data.push_back(code);

  }else if(name.compare("beq ") == 0){

    string code = "0011";
    if(hasPool2){
      code += "10011111";
    }else{
      code += "00010000";
    }
    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op1, 4);
    code += getBits(op2, 4);
    code += currentOperandOffset;
    sec->second.data.push_back(code);

  }else if(name.compare("bne ") == 0){

    string code = "0011";
    if(hasPool2){
      code += "10101111";
    }else{
      code += "00100000";
    }
    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op1, 4);
    code += getBits(op2, 4);
    code += currentOperandOffset;
    sec->second.data.push_back(code);

  }else if(name.compare("bgt ") == 0){

    string code = "0011";
    if(hasPool2){
      code += "10111111";
    }else{
      code += "00110000";
    }
    op1 = op1.substr(2);
    op2 = op2.substr(2);
    code += getBits(op1, 4);
    code += getBits(op2, 4);
    code += currentOperandOffset;
    sec->second.data.push_back(code);

  }else if(name.compare("jmp ") == 0){

    string code = "0011";
    if(hasPool2){
      code += "10001111";
    }else{
      code += "00000000";
    }
    code += "00000000";
    code += currentOperandOffset;
    sec->second.data.push_back(code);

  }else if(name.compare("call ") == 0){

    string code = "0010";
    if(hasPool2){
      code += "00011111";
    }else{
      code += "00000000";
    }
    code += "00000000";
    code += currentOperandOffset;
    sec->second.data.push_back(code);

  }else if(name.compare("ld ") == 0){

    string code = "1001";
    cout << "MAJMUN" << endl;
    
    if(inParrens){

      code += "0010"; //modifikator
      op1 = op1.substr(2);
      code += getBits(op1, 4); // A - r1
      code += parrensReg; //B - r2
      code += "0000"; //C - r0 
      if(parrensHex != ""){
        code += parrensHex; // D
      }else{
        code += "000000000000";
      }     
      sec->second.data.push_back(code);

    }else if(hasPool2){
      code += "0010"; //modifikator
      op1 = op1.substr(2);
      code += getBits(op1, 4); // A - r1
      code += "1111"; //B - pc
      code += "0000"; //C - r0 
      code += currentOperandOffset; //pomeraj
      sec->second.data.push_back(code);

      if(!inOprString){
        currentSectionSize += 4;
        sec->second.offsets.push_back(currentSectionSize);
        code = "1001";
        code += "0010"; //modifikator
        code += getBits(op1, 4); //A 
        code += getBits(op1, 4); //B 
        code += "0000"; //C
        code += "000000000000"; //D - pomeraj je 0
        sec->second.data.push_back(code);
      }

      cout << "MAJMUNCINA: " << code << endl;

    }else{ //unutar 12b
      code += "0001"; //modifikator
      op1 = op1.substr(2);
      code += getBits(op1, 4); // A - r1
      code += "0000"; //B 
      code += "0000"; //C - r0 
      code += currentOperandOffset; //pomeraj 
      sec->second.data.push_back(code);
    }

  }else if(name.compare("st ") == 0){
    string code = "1000";
    
    if(inParrens){
      code += "0000"; //modifikator
      code += parrensReg; //A - r2
      code += "0000"; //B - r0 
      op1 = op1.substr(2);
      code += getBits(op1, 4); //C - r1 
      if(parrensHex != ""){
        code += parrensHex; // D
      }else{
        code += "000000000000";
      }
      sec->second.data.push_back(code);

    }else{
      code += "0010"; //modifikator
      code += "1111"; //A - pc 
      code += "0000"; //B - r0 
      op1 = op1.substr(2);
      code += getBits(op1, 4);      //C
      code += currentOperandOffset; //D
      sec->second.data.push_back(code);
    }

  }else if(name.compare(".skip ") == 0){

    currentSectionSize = currentSectionSize + skipNum - 4;
    string code = "";
    for(int i = 0; i < skipNum; i++){
      code += "0000";
    }
    sec->second.data.push_back(code);

    skipNum = -1;

  }else if(name == ".word "){
    //...
  }else{
    cout << "ERROR: Non instruction:" << name << "I" << endl;
    exit(1);
  }

  if(name != ".word "){
    currentSectionSize += 4;
    fileOffset += 4;
  }
  
  currentOperandOffset = " OFFSET ";
  inParrens = false;
  parrensReg = "";
  parrensHex = "";
  inOprString = false;
}

void Assembler::getOperand(string name, string type){

  PoolOfLiterals p;
  
  map<string,Section>::iterator sec = sections.find(currentSectionName);
  if(type.compare("opr_dec") == 0){
    name.erase(0, 1);
    int num = stoi(name);

    if(num <= 2047 || num >= -2048){
      currentOperandOffset = getBits(name, 12);
      hasPool2 = false;
    }else{

      p.isSymbol = false;
      p.symbolAddress = sec->second.poolSize;
      p.symbolName = "opr_dec";
      p.symbolValue = num;
      poolVector.push_back(p);
      
      sec->second.hasPool = true;
      sec->second.poolSize += 4;

      currentOperandOffset = "ZAMENITIOVO!";
      hasPool2 = true;

      sectionSizes.push_back(currentSectionSize);
      poolSizes.push_back(p.symbolAddress);
    }

  }else if(type.compare("opr_hex") == 0){

    name.erase(0, 3);
    string zeros;
    for(int i = 0; i < 8 - name.size(); i++){
      zeros += "0";
    }
    name = zeros + name;
    long long num = stoll(name, nullptr, 16);

    if(num <= 4095){
      currentOperandOffset = getBits(to_string(num), 12);
      hasPool2 = false;
    }else{

      p.isSymbol = false;
      p.symbolAddress = sec->second.poolSize;
      p.symbolName = "opr_hex";
      p.symbolValue = num;
      poolVector.push_back(p);

      sec->second.hasPool = true;
      sec->second.poolSize += 4;

      currentOperandOffset = "ZAMENITIOVO!";
      hasPool2 = true;
      inOprString = true; 

      sectionSizes.push_back(currentSectionSize);
      poolSizes.push_back(p.symbolAddress);
    }

  }else if(type.compare("opr_string") == 0){

    name.erase(0, 1);
    
    p.isSymbol = true;
    p.symbolAddress = sec->second.poolSize;
    p.symbolName = name;
    p.symbolValue = 0;

    poolVector.push_back(p);
    auto sec = sections.find(currentSectionName);
    sec->second.hasPool = true;
    sec->second.poolSize += 4;

    currentOperandOffset = "ZAMENITIOVO!";
    hasPool2 = true;
    inOprString = true;

    sectionSizes.push_back(currentSectionSize);
    poolSizes.push_back(p.symbolAddress);

  }else{

    p.isSymbol = true;
    p.symbolAddress = sec->second.poolSize;
    p.symbolName = name;
    p.symbolValue = 0;
    poolVector.push_back(p);

    sec->second.hasPool = true;
    sec->second.poolSize += 4;

    currentOperandOffset = "ZAMENITIOVO!";
    hasPool2 = true;
    
    sectionSizes.push_back(currentSectionSize);
    poolSizes.push_back(p.symbolAddress);
  }

}

void Assembler::getLiteral(string name, string type){

  PoolOfLiterals p;

  map<string,Section>::iterator sec = sections.find(currentSectionName);
  if(type == "dec"){

    int num = stoi(name);
    skipWordNum = num;
    if(num <= 2047 && num >= -2048){
      currentOperandOffset = getBits(name, 12);
      hasPool2 = false;
      if(currentInstruction == ".skip "){
        skipNum = num;
      } else
      if(currentInstruction == ".word "){
        sec->second.offsets.push_back(currentSectionSize);
        string data = "00000000000000000000" + currentOperandOffset;
        sec->second.data.push_back(data);
        currentSectionSize += 4;
        fileOffset += 4;
      }
    }else{

      p.isSymbol = false;
      p.symbolAddress = sec->second.poolSize;
      p.symbolName = "dec"; 
      p.symbolValue = num;
      poolVector.push_back(p);

      sec->second.hasPool = true;
      sec->second.poolSize += 4;

      currentOperandOffset = "ZAMENITIOVO!";                            
      hasPool2 = true;
      
      sectionSizes.push_back(currentSectionSize);
      poolSizes.push_back(p.symbolAddress);
    }

  }else if(type == "hex"){
    cout << "ASDASD" << endl;
    name.erase(0, 2);
    string zeros;
    for(int i = 0; i < 8 - name.size(); i++){
      zeros += "0";
    }
    name = zeros + name;
    long long num = stoll(name, nullptr, 16);
    skipWordNum = num;

    if(num <= 4095){
      currentOperandOffset = getBits(to_string(num), 12);
      hasPool2 = false;
      if(currentInstruction == ".skip "){
        skipNum = num;
      } else
      if(currentInstruction == ".word "){
        sec->second.offsets.push_back(currentSectionSize);
        string data = "00000000000000000000" + currentOperandOffset;
        sec->second.data.push_back(data);
        currentSectionSize += 4;
        fileOffset += 4;
      }
    }else{

      p.isSymbol = false;
      p.symbolAddress = sec->second.poolSize;
      p.symbolName = "hex"; 
      p.symbolValue = num;
      poolVector.push_back(p);

      sec->second.hasPool = true;
      sec->second.poolSize += 4;

      currentOperandOffset = "ZAMENITIOVO!";                           
      hasPool2 = true;
      
      sectionSizes.push_back(currentSectionSize);
      poolSizes.push_back(p.symbolAddress);
    }

  }else{

    p.isSymbol = true;
    p.symbolAddress = sec->second.poolSize;
    p.symbolName = name;
    p.symbolValue = 0;
    poolVector.push_back(p);
    
    sec->second.hasPool = true;
    sec->second.poolSize += 4;

    currentOperandOffset = "ZAMENITIOVO!";                            
    hasPool2 = true;
   
    sectionSizes.push_back(currentSectionSize);
    poolSizes.push_back(p.symbolAddress);
  }
}

void Assembler::getParrensBody(string name, string type){

  PoolOfLiterals p;
  
  map<string,Section>::iterator sec = sections.find(currentSectionName);
  inParrens = true;

  if(type == "gpr_reg"){
    
    if(name == "%sp") name = "%r14";
    if(name == "%pc") name = "%r15";
    name = name.substr(2);
    parrensReg = getBits(name, 4);

  }else if(type == "hex"){
    
    name.erase(0, 2);
    string zeros;

    for(int i = 0; i < 8 - name.size(); i++){
      zeros += "0";
    }

    name = zeros + name;
    long long num = stoll(name, nullptr, 16);

    if(num <= 4095){
      parrensHex = getBits(to_string(num), 12);
      
    }else{

      p.isSymbol = false;
      p.symbolAddress = sec->second.poolSize;
      p.symbolName = "hex";
      p.symbolValue = num;
      poolVector.push_back(p);

      sec->second.hasPool = true;
      sec->second.poolSize += 4;
      ///////////////////////////

      cout << "ERROR: HEX num out of limit!!!" << endl;
      exit(1);
    }
  }
}

void Assembler::addPoolToSec(){

  for(const auto& section : sections) {
    string secName = section.first;
    Section sec = section.second;

    for(int i = 0; i < pools[secName].size(); i++){
      sec.offsets.push_back(pools[secName].at(i).symbolAddress);
      string strValue = bitset<32>(pools[secName].at(i).symbolValue).to_string();
      sec.data.push_back(strValue);
    }

    sections[secName].offsets = sec.offsets;
    sections[secName].data = sec.data;

  }
}

//////////////////POMOCNE FUNKCIJE/////////////////////

bool Assembler::inTable(string name){
  auto it = symbols.find(name);
  if (it != symbols.end())
      return true;

  return false; 
}

string Assembler::getBits(const string& stringInt, int nBits) {
    int intValue = stoi(stringInt);
    
    if (intValue >= (1 << nBits)) {
        cout << "ERROR: Integer value exceeds " << nBits << " bits representation." << endl;
        exit(1);
    }

    bitset<32> bits(intValue);
    string bitString = bits.to_string();

    return bitString.substr(32 - nBits);
}

string Assembler::getOperandOffset(){
  return "OFFSET";
}

void Assembler::displayOffsetsTable(const map<string, OffsetAlter>& symbolMap){
  cout << "       -------------------------------OFFSETS-----------------------------------" << endl;
  cout << setw(15) << "Section" << setw(15) << "SectionSize" << setw(15) << "PoolSize" << endl;

  for (const auto& entry : symbolMap) {
    const vector<int>& sectionSize = entry.second.sectionSize;
    const vector<int>& poolSize = entry.second.poolSize;

    for (int i = 0; i < sectionSize.size(); i++) {
        cout << setw(15) << entry.first << setw(15) << sectionSize.at(i) << setw(15) << poolSize.at(i) << endl;
    }
  }
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

void Assembler::displayRelocationTable(const map<string, vector<RealocationEntry>>& symbolMap){

  cout << "    --------------------------RELOCATIONS--------------------------" << std::endl;
  cout << setw(15) << "Section" << setw(15) << "Offset" << setw(20) << "Symbol"
       << setw(15) << "Addent" << endl;

  for (const auto& entry : symbolMap) {
    const vector<RealocationEntry>& relocations = entry.second;

    for (const auto& relocation : relocations) {
      cout << setw(15) << relocation.section << setw(15) << relocation.offset
           << setw(20) << relocation.symbol << setw(15) << relocation.addent << endl;
    }
  }

  cout << "\n" << endl;
}

void Assembler::makeOutputFile(){

  ofstream file(fileOutput, ios::out | ios::binary);

  for (const auto& entry : relocations) {
    size_t relocationSize = entry.second.size();
    file.write(reinterpret_cast<const char*>(&relocationSize), sizeof(relocationSize));
    
    for (const auto& realocation : entry.second) {
      size_t nameLength = entry.first.size();
      file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
      file.write(entry.first.c_str(), nameLength);
      file.write(reinterpret_cast<const char*>(&realocation.offset), sizeof(realocation.offset));
      size_t symbolLength = realocation.symbol.size();
      file.write(reinterpret_cast<const char*>(&symbolLength), sizeof(symbolLength));
      file.write(realocation.symbol.c_str(), symbolLength);
      file.write(reinterpret_cast<const char*>(&realocation.addent), sizeof(realocation.addent));
    }
  }
  
  file.write("===\n", 4);
  
  for (const auto& entry : sections) {
    const Section& section = entry.second;
    
    size_t nameLength = section.name.size();
    file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    file.write(section.name.c_str(), nameLength);
    file.write(reinterpret_cast<const char*>(&section.size), sizeof(section.size));
    file.write(reinterpret_cast<const char*>(&section.serialNum), sizeof(section.serialNum));
    file.write(reinterpret_cast<const char*>(&section.hasPool), sizeof(section.hasPool));
    file.write(reinterpret_cast<const char*>(&section.poolSize), sizeof(section.poolSize));
    
    size_t offsetsSize = section.offsets.size();
    file.write(reinterpret_cast<const char*>(&offsetsSize), sizeof(offsetsSize));
    for (long long offset : section.offsets) {
      file.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }
    
    size_t dataSize = section.data.size();
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    for (const std::string& data : section.data) {
      size_t dataLength = data.size();
      file.write(reinterpret_cast<const char*>(&dataLength), sizeof(dataLength));
      file.write(data.c_str(), dataLength);
    }
  }
  
  file.write("===\n", 4);
  
  size_t symbolsSize = symbols.size();
  file.write(reinterpret_cast<const char*>(&symbolsSize), sizeof(symbolsSize));
  for (const auto& entry : symbols) {
    const Symbol& symbol = entry.second;
    
    size_t nameLength = symbol.name.size();
    file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    file.write(symbol.name.c_str(), nameLength);
    file.write(reinterpret_cast<const char*>(&symbol.serialNum), sizeof(symbol.serialNum));
    file.write(reinterpret_cast<const char*>(&symbol.value), sizeof(symbol.value));
    file.write(reinterpret_cast<const char*>(&symbol.isLocal), sizeof(symbol.isLocal));
    
    size_t sectionLength = symbol.section.size();
    file.write(reinterpret_cast<const char*>(&sectionLength), sizeof(sectionLength));
    file.write(symbol.section.c_str(), sectionLength);
    file.write(reinterpret_cast<const char*>(&symbol.isSection), sizeof(symbol.isSection));
    file.write(reinterpret_cast<const char*>(&symbol.offset), sizeof(symbol.offset));
  }

  file.close();
  createTextFile();
}

void Assembler::createTextFile(){
  
  fileOutput.erase(fileOutput.size() - 2);
  ofstream file(fileOutput + ".txt");
  for (const auto& entry : relocations) {
    if( entry.second.size() > 0)
      file << entry.second.size() << '\n';

    for (const auto& realocation : entry.second) {
      file << entry.first << "," << realocation.offset << ',' << realocation.symbol << ',' << realocation.addent << '\n';
    }
  }

  file << "===\n"; // kraj relokacija
  
  for (const auto& entry : sections) {

    const Section& section = entry.second;

    file << section.name << "," << section.size << ",";
    file << section.serialNum << "," << section.hasPool << ",";
    file << section.poolSize << "\n";
    
    file << section.offsets.size() << "\n";
    for (long long offset : section.offsets) {
        file << offset << " ";
    }
    file << "\n";

    for (const std::string& data : section.data) {
        file << data << " ";
    }
    file << "\n";
  }

  file << "===\n"; // kraj sekcija

  file << symbols.size() << "\n";
  for (const auto& entry : symbols) {

    const Symbol& symbol = entry.second;

    file << symbol.name << "," << symbol.serialNum << ",";
    file << symbol.value << "," << symbol.isLocal << ",";
    file << symbol.section << "," << symbol.isSection << ",";
    file << symbol.offset << "\n";
  
  }  

  file.close();

}

string srcFolder = "./test/nivo-a/";

int main(int argc, char* argv[]){

  if(argc != 4){
    cout << "INPUT ERROR: Nedovaoljan broj argumenata za asembliranje!!!";
    exit(1);
  }
  
  if(strcmp(argv[1], "-o")){
    cout << "INPUT ERROR: Lose formatirana funkcija!!!";
    exit(1);
  }
  size_t lengthOutput = strlen(argv[2]);
  size_t lengthInput = strlen(argv[3]);

  char lastTwoOutput[3];
  char lastTwoInput[3];
  if (lengthOutput >= 2 && lengthInput >= 2) {
    strncpy(lastTwoOutput, argv[2] + lengthOutput - 2, 2);
    lastTwoOutput[2] = '\0';
    strncpy(lastTwoInput, argv[3] + lengthInput - 2, 2);
    lastTwoInput[2] = '\0';
  }else{
    cout << "INPUT ERROR: lose formatirani fajlovi duzina!!!";
    exit(1);
  }

  if(strcmp(lastTwoOutput, ".o")){
    cout << "INPUT ERROR: lose formatirani fajl izgled output!!!";
    exit(1);
  }

  if(strcmp(lastTwoInput, ".s")){
    cout << "INPUT ERROR: lose formatirani fajl izgled input!!!";
    exit(1);
  }

  Assembler::passFile(srcFolder + argv[3], argv[2]);

  printf("Prosao ceo asembler bez greske :)\n");

  return 1;
}