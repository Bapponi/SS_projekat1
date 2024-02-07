#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
#include <iomanip> 

#include "../inc/emulator.hpp"

using namespace std;

vector<Code> Emulator::codes;
string Emulator::inputFile;
string Emulator::currentInstruction;
map<int, string> Emulator::bytes;

int Emulator::pc;
int Emulator::sp;
bool Emulator::executing;
bool Emulator::error;
string Emulator::inst1;
string Emulator::instM;
string Emulator::instA;
string Emulator::instB;
string Emulator::instC;
string Emulator::instD;
string Emulator::variation;

void Emulator::init(string fileName){
  codes.clear();
  inputFile = fileName;
  currentInstruction = "";
  bytes.clear();

  pc = 0;
  sp = 0xFFFF0000;
  executing = true;
  error = false;
  inst1 = "";
  instM = "";
  instA = "";
  instB = "";
  instC = "";
  instD = "";
  variation = "";
}

void Emulator::startEmulator(){
  getTextFile(inputFile);
  setupBytes();
  programExecute();  
  displayCode(codes);
}

void Emulator::getTextFile(string fileName){

  fileName.erase(fileName.size() - 4);

  ifstream file(fileName + ".txt");
  
  string line;

  while(getline(file, line)){
    
    if (line == "===") {
      break; 
    }

    vector<string> vektor = splitString(line, ',');
    Code c;
    c.address = stoi(vektor.at(0));
    c.addressHex = decimalToHex(stoi(vektor.at(0)));
    c.data = vektor.at(1);
    c.size = stoi(vektor.at(2));
    c.dataHex = binaryToHex(vektor.at(1), stoi(vektor.at(2)));
    codes.push_back(c);

  }
}

void Emulator::setupBytes(){

  for(int i = 0; i < codes.size(); i++){
    
    int size = codes.at(i).dataHex.length();
    
    for(int j = 0; j < size / 2; j++){
      bytes.insert(make_pair(i*4 + j, codes.at(i).dataHex.substr(j*2, 2)));
    }
  }

}

void Emulator::programExecute(){

  int exeNum = 0;
  while(executing){

    if(exeNum++ >= 50) break;
    instructionStart();
    pc += 4;
  }
}

void Emulator::instructionStart(){

  setInstructionReg();

  if(inst1 == "0"){
  
    currentInstruction = "halt";
    if(instM != "0" || instA != "0" || instB != "0" || instC != "0" || instD != "000"){
      error = true;
    }
  
  }else if(inst1 == "1"){

    currentInstruction = "int";
    if(instM != "0" || instA != "0" || instB != "0" || instC != "0" || instD != "000"){
      error = true;
    }
  
  }else if(inst1 == "2"){

    currentInstruction = "call";

    if(instM == "0") variation = "noPool";        
    else if(instM == "1") variation = "yesPool";
    else error = true;    

    if(instC != "0") error = true;  

  }else if(inst1 == "3"){
    
    if(instM == "0"){
      currentInstruction = "jmp";
      variation = "noPool";

      if(instB != "0" || instC != "0") error = true;         
    }
    else if(instM == "1"){
      currentInstruction = "beq";
      variation = "noPool";
    }
    else if(instM == "2"){
      currentInstruction = "bne";
      variation = "noPool";
    }
    else if(instM == "3"){
      currentInstruction = "bgt";
      variation = "noPool";
    }
    else if(instM == "8"){
      currentInstruction = "jmp";
      variation = "yesPool";

      if(instB != "0" || instC != "0") error = true;         
    }
    else if(instM == "9"){
      currentInstruction = "beq";
      variation = "yesPool";
    }
    else if(instM == "a"){
      currentInstruction = "bne";
      variation = "yesPool";
    }
    else if(instM == "b"){
      currentInstruction = "bgt";
      "yesPool";
    }
    else error = true;

  }else if(inst1 == "4"){

    currentInstruction = "xchg";
    if(instM != "0" || instA != "0" || instD != "000"){
      error = true;
    }

  }else if(inst1 == "5"){

    if(instM == "0") currentInstruction = "add";
    else if(instM == "1") currentInstruction = "sub";
    else if(instM == "2") currentInstruction = "mul";
    else if(instM == "3") currentInstruction = "div";
    else error = true;

    if(instD != "000") error = true;

  }else if(inst1 == "6"){
    if(instM == "0"){
      currentInstruction = "not";
      if(instC != "0") error = true;
    } 
    else if(instM == "1") currentInstruction = "and";
    else if(instM == "2") currentInstruction = "or";
    else if(instM == "3") currentInstruction = "xor";
    else error = true;

    if(instD != "000") error = true;

  }else if(inst1 == "7"){

    if(instM == "0") currentInstruction = "shl";
    else if(instM == "1") currentInstruction = "shr";
    else error = true;

    if(instD != "000") error = true;

  }else if(inst1 == "8"){

    //izmeniti
    currentInstruction = "st";
    if(instM == "0") variation = "noPool";
    else if(instM == "1") variation = "notKnow";
    else if(instM == "2") variation = "yesPool";
    else error = true;

  }else if(inst1 == "9"){

    //izmeniti
    currentInstruction = "ld";
    if(instM == "0") variation = "noPool";
    else if(instM == "1") variation = "notKnow";
    else if(instM == "2") variation = "yesPool";
    else if(instM == "3") variation = "yesPool";
    else if(instM == "4") variation = "yesPool";
    else if(instM == "6") variation = "yesPool";
    else error = true;

  }else{
    cout << "\nERROR: Bad instruction" << endl;
    error = true;
  }
}

void Emulator::setInstructionReg(){
  inst1 = bytes[pc].at(0);
  instM = bytes[pc].at(1);
  instA = bytes[pc + 1].at(0);
  instB = bytes[pc + 1].at(1);
  instC = bytes[pc + 2].at(0);
  instD = bytes[pc + 2].at(1) + bytes[pc + 3];
}

void Emulator::executeInstruction(){

  string name = currentInstruction;

  if(name == "halt"){

  }else if(name == "iret"){

  }else if(name == "ret"){

  }else if(name == "pop"){

  }else if(name == "xchg"){

  }else if(name == "add"){

  }else if(name == "sub"){

  }else if(name == "mul"){

  }else if(name == "div"){

  }else if(name == "and"){

  }else if(name == "or"){

  }else if(name == "xor"){

  }else if(name == "shl"){

  }else if(name  == "shr"){

  }else if(name == "csrrd"){

  }else if(name  == "csrwr"){

  }else if(name == "beq"){

  }else if(name == "bne"){

  }else if(name == "bgt"){

  }else if(name == "ld"){

  }else if(name == "st"){
    
  }else{
    cout << "ERROR: Non instruction: " << name <<  endl;
    exit(1);
  }
}

////////////////////////POMOCNE FUNKCIJE/////////////////////////////

vector<string> Emulator::splitString(const string& input, char delimiter) {
  
  vector<string> result;
  size_t start = 0;
  size_t end = input.find(delimiter);

  while (end != string::npos) {
      result.push_back(input.substr(start, end - start));
      start = end + 1;
      end = input.find(delimiter, start);
  }

  result.push_back(input.substr(start));

  return result;

}

string Emulator::binaryToHex(const string& binaryString, int size) {

  bitset<32> bitset(binaryString);
  unsigned long hexValue = bitset.to_ulong();

  stringstream ss;
  ss << hex << setw(size*2) << setfill('0') << hexValue;
  return ss.str();

}

string Emulator::decimalToHex(int decimalValue) {
  
  stringstream ss;
  ss << hex << decimalValue;
  return ss.str();

}

void Emulator::displayCode(const vector<Code>& codeVector){
  cout << "------------------------------------Codes-------------------------------------" << endl;
  cout << setw(15) << "Address" << setw(36) << "Data" 
       << setw(15) << "AddressHex" << setw(9) << "DataHex" << endl;

  for (int i = 0; i < codeVector.size(); i++) {
    cout << setw(15) << codeVector.at(i).address  << ":" << setw(36) << codeVector.at(i).data 
         << setw(15) << codeVector.at(i).addressHex << ":" << setw(9) << codeVector.at(i).dataHex << endl;
  }

  cout << "\n" << endl;
}

int main(int argc, char* argv[]){

  if(argc != 2){
    cout << "ERROR: incorect input instruction" << endl;
    exit(1);
  }

  size_t lengthInput = strlen(argv[1]);
  
  char lastThreeInput[4];
  if (lengthInput >= 2) {
    strncpy(lastThreeInput, argv[1] + lengthInput - 3, 3);
    lastThreeInput[4] = '\0';
  }else{
    cout << "INPUT ERROR: lose formatirani fajlovi duzina!!!";
    exit(1);
  }

  if(strcmp(lastThreeInput, "hex")){
    cout << "INPUT ERROR: lose formatirani fajl izgled output!!!";
    exit(1);
  }
    
  Emulator::init(argv[1]);
  Emulator::startEmulator();

  printf("Prosao emulator bez greske :)\n");

  return 1;
}