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
map<long long, string> Emulator::bytes;
vector<int> Emulator::regs;
vector<int> Emulator::csr; 

unsigned int Emulator::regM; // MAJMUN: obrisi potencijalno
unsigned int Emulator::startAddress;
int Emulator::pc;
int Emulator::sp;
int Emulator::status;
int Emulator::handler;
int Emulator::cause;
bool Emulator::executing;
bool Emulator::error;
int Emulator::inst1;
int Emulator::instM;
int Emulator::instA;
int Emulator::instB;
int Emulator::instC;
int Emulator::instD;
string Emulator::variation;

int Emulator::TR;
int Emulator::TL;
int Emulator::I;

void Emulator::init(string fileName){
  codes.clear();
  inputFile = fileName;
  currentInstruction = "";
  bytes.clear();
  regs.clear();
  csr.clear();

  for(int i = 0; i < 16; i++)
    regs.push_back(0);

  for(int i = 0; i < 3; i++)
    csr.push_back(0);

  status = 0;
  handler = 1;
  cause = 2;

  regM = 0xFFFF0000;   // MAJMUN: obrisi potencijalno
  startAddress = 0x40000000;
  pc = 15;
  sp = 14;

  regs[pc] = startAddress;
  regs[sp] = regM;     // MAJMUN: obrisi potencijalno
  csr[handler] = regM; // MAJMUN: obrisi potencijalno
  executing = true;
  error = false;
  inst1 = -1;
  instM = -1;
  instA = -1;
  instB = -1;
  instC = -1;
  instD = -1;
  variation = "";

  TR = 1; // MAJMUN: obrisi potencijalno
  TL = 2; // MAJMUN: obrisi potencijalno
  I = 4;  // MAJMUN: obrisi potencijalno
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
    c.address = stoll("0x" + vektor.at(3), nullptr, 16);
    c.addressHex = vektor.at(3);
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
      // bytes.insert(make_pair(codes.at(i).address + j, codes.at(i).dataHex.substr(j*2, 2)));
      bytes[codes.at(i).address + j] = codes.at(i).dataHex.substr(j*2, 2);
    }
  }

  // bytes[55] = "90";
  // cout << "BAJT: " << bytes[55] << endl;

}

void Emulator::startEmulator(){
  cout << "START" << endl;
  getTextFile(inputFile);
  displayCode(codes);
  setupBytes();
  programExecute();

  cout << "PROGRAM EXECUTION FINISHED WITH THIS STATE:" << endl;
  showCurrentState();  
}

void Emulator::programExecute(){
  
  while(executing){
    
    // showCurrentState();
    instructionStart();

  }
}

void Emulator::instructionStart(){

  setInstructionReg();

  if(inst1 == 0){
  
    currentInstruction = "halt";
    if(instM != 0 || instA != 0 || instB != 0 || instC != 0 || instD != 0){
      error = true;
    }
    executing = false;
  
  }else if(inst1 == 1){
    
    //nesmes da radis int ako su prekidi maskirani - ovo je bitno za tajmer
    currentInstruction = "int";
    if(instM != 0 || instA != 0 || instB != 0 || instC != 0 || instD != 0){
      error = true;
    }
    startInterrupt();
  
  }else if(inst1 == 2){

    currentInstruction = "call";

    if(instM == 0){
      regs[pc] = regs[instA] + regs[instB] + instD;
    }        
    else if(instM == 1){
      regs[pc] = getValueFromAddress(regs[instA] + regs[instB] + instD);
    }
    else error = true;    

    if(instC != 0) error = true;  

  }else if(inst1 == 3){
    
    if(instM == 0){
      currentInstruction = "jmp";
      
      regs[pc] = regs[instA] + instD;

      if(instB != 0 || instC != 0) error = true;         
    }
    else if(instM == 1){
      currentInstruction = "beq";
      
      if(regs[instB] == regs[instC])
        regs[pc] = regs[instA] + instD;

    }
    else if(instM == 2){
      currentInstruction = "bne";
      
      if(regs[instB] != regs[instC])
        regs[pc] = regs[instA] + instD;

    }
    else if(instM == 3){
      currentInstruction = "bgt";
      
      if(regs[instB] > regs[instC])
        regs[pc] = regs[instA] + instD;

    }
    else if(instM == 8){
      currentInstruction = "jmp";
      regs[pc] = getValueFromAddress((regs[instA] + instD));

      if(instB != 0 || instC != 0) error = true;         
    }
    else if(instM == 9){
      currentInstruction = "beq";
      
      if(regs[instB] == regs[instC])
        regs[pc] = getValueFromAddress((regs[instA] + instD));

    }
    else if(instM == 0xa){
      currentInstruction = "bne";
      
      if(regs[instB] != regs[instC])
        regs[pc] = getValueFromAddress((regs[instA] + instD));

    }
    else if(instM == 0xb){
      currentInstruction = "bgt";
      
      if(regs[instB] > regs[instC])
        regs[pc] = getValueFromAddress((regs[instA] + instD));

    }
    else error = true;

  }else if(inst1 == 4){

    currentInstruction = "xchg";

    int help = regs[instA];
    regs[instA] = regs[instB];
    regs[instB] = help;

    if(instM != 0 || instA != 0 || instD != 0){
      error = true;
    }

  }else if(inst1 == 5){

    if(instM == 0){
      currentInstruction = "add";
      regs[instA] = regs[instB] + regs[instC];
    } 
    else if(instM == 1){
      currentInstruction = "sub";
      regs[instA] = regs[instB] - regs[instC];
    } 
    else if(instM == 2){
      currentInstruction = "mul";
      regs[instA] = regs[instB] * regs[instC];
    } 
    else if(instM == 3){
      currentInstruction = "div";

      if(instC != 0)
        regs[instA] = regs[instB] / regs[instC];
      else
        error = true;

    } 
    else error = true;

    if(instD != 0) error = true;

  }else if(inst1 == 6){
    if(instM == 0){
      currentInstruction = "not";

      regs[instA] = !regs[instB];

      if(instC != 0) error = true;
    } 
    else if(instM == 1){
      currentInstruction = "and";
      regs[instA] = regs[instB] & regs[instC];
    }
    else if(instM == 2){
      currentInstruction = "or";
      regs[instA] = regs[instB] | regs[instC];
    }
    else if(instM == 3){
      currentInstruction = "xor";
      regs[instA] = regs[instB] ^ regs[instC];
    }
    else error = true;

    if(instD != 0) error = true;

  }else if(inst1 == 7){

    if(instM == 0){
      currentInstruction = "shl";
      regs[instA] = regs[instB] << regs[instC];
    }
    else if(instM == 1){
      currentInstruction = "shr";
      regs[instA] = regs[instB] >> regs[instC];
    }
    else error = true;

    if(instD != 0) error = true;

  }else if(inst1 == 8){

    currentInstruction = "st";
    if(instM == 0){

      int address = regs[instA] + regs[instB] + instD;
      int value = regs[instC];
      setValueOnAddress(address, value);

    }
    else if(instM == 1){

      pushOnStack(regs[instC]);

    }else if(instM == 2){

      int address = getValueFromAddress(regs[instA] + regs[instB] + instB);
      int value = regs[instC];
      setValueOnAddress(address, value);

    }
    else error = true;

  }else if(inst1 == 9){

    currentInstruction = "ld";
    if(instM == 0){
      regs[instA] = csr[instB];
    }
    else if(instM == 1){
      regs[instA] = regs[instB] + instD;
    }
    else if(instM == 2){

      int address = regs[instB] + regs[instC] + instD;
      regs[instA] = getValueFromAddress(address);

    }
    else if(instM == 3){
      regs[instA] = popFromStack();
    }
    else if(instM == 4){
      csr[instA] = regs[instB];
    }
    else if(instM == 5){
      csr[instA] = csr[instB] | instD;
    }
    else if(instM == 6){

      int address = regs[instB] + regs[instC] + instD;
      csr[instA] = getValueFromAddress(address);

    }
    else if(instM == 7){

      int address = regs[instB];
      csr[instA] = getValueFromAddress(address);
      regs[instB] = regs[instB] + instD;

    }
    else if(instM == 0xf){
      currentInstruction = "iret"; // MAJMUN: potencijalno problem
      // csr[status] = getValueFromAddress(regs[sp]) + 4; //ako pokazuje na poslednju zauzetu
      // regs[pc] = popFromStack();
      // popFromStack(); //
      csr[status] = popFromStack(); //posto je atomicno, ovo je ok
    }else error = true;

  }else{
    cout << "\nERROR: Bad instruction" << endl;
    error = true;
  }

  // showCurrentState();

  if(error){
    cout << "ERROR: something bad with instruction: " << currentInstruction << endl;
    exit(1);
  }
}

void Emulator::setInstructionReg(){


  string pom = string(1, bytes[regs[pc]].at(0));
  inst1 = stoi("0x" + pom, nullptr, 16);

  string pom1 = string(1, bytes[regs[pc]].at(1));
  instM = stoi("0x" + pom1, nullptr, 16);

  string pom2 = string(1, bytes[regs[pc] + 1].at(0));
  instA = stoi("0x" + pom2, nullptr, 16);

  string pom3 = string(1, bytes[regs[pc] + 1].at(1));
  instB = stoi("0x" + pom3, nullptr, 16);

  string pom4 = string(1, bytes[regs[pc] + 2].at(0));
  instC = stoi("0x" + pom4, nullptr, 16);

  string pom5 = string(1, bytes[regs[pc] + 2].at(1));
  instD = stoi("0x" + pom5, nullptr, 16) * 256 + stoi("0x" + bytes[regs[pc] + 3], nullptr, 16);

  showCurrentState();
  regs[pc] += 4;

}

long long Emulator::getValueFromAddress(int address){
  cout << "Adresa: " << address  << "  Vrednost: " << bytes[address] + bytes[address + 1] + bytes[address + 2] + bytes[address + 3] << endl;
  return stoll("0x" + bytes[address] + bytes[address + 1] + bytes[address + 2] + bytes[address + 3], nullptr, 16); 
}

string Emulator::getStringFromAddress(int address){
  return bytes[address] + bytes[address + 1] + bytes[address + 2] + bytes[address + 3]; 
}

void Emulator::setValueOnAddress(int address, int value){

  string pom = decimalToHex(value);

  bytes[address] = pom.substr(0, 2);
  bytes[address + 1] = pom.substr(2, 2);
  bytes[address + 2] = pom.substr(4, 2);
  bytes[address + 3] = pom.substr(6, 2);

}

int Emulator::popFromStack(){
  
  int value = getValueFromAddress(regs[sp]);

  cout << "From stack: " << value << endl;
  regs[sp] = regs[sp] + 4;
  
  return value;
}

void Emulator::pushOnStack(int value){
  regs[sp] = regs[sp] - 4;
  setValueOnAddress(regs[sp], value);
}

void Emulator::startInterrupt(){
  
  if(csr[handler] == regM) // MAJMUN: obrisi potencijalno
    return;

    pushOnStack(csr[status]);

    pushOnStack(regs[pc]);

    regs[pc] = csr[handler];

    csr[status] = I; //ne treba maskiranje prekida povedi racuna // MAJMUN: obrisi potencijalno
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
  ss << hex << setw(8) << setfill('0') << decimalValue;
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

void Emulator::showCurrentState(){

  cout << "Current instruction name: " << currentInstruction << endl;
  cout << "-----------------------------------------------------------" << endl;

  cout << "\nInst1: " << inst1 << "  InstM: "<< instM << "  InstA: " << instA << "  InstB: " << instB << "  InstC: " << instC << "  InstD: " << instD << endl;

  for(int i = 0; i < regs.size() - 2; i = i + 2){
    cout << "REG" << i << ": 0x" << setfill('0') << setw(8) << regs[i] << "    ";
    cout << "REG" << i + 1 << ": 0x" << setfill('0') << setw(8) << regs[i + 1] << endl;
  }
  cout << "\nSP: 0x" << setfill('0') << setw(8) << hex << regs[14];
  cout << " PC: 0x" << setfill('0') << setw(8) << hex << regs[15] << endl;
  cout << "Status: 0x" << setfill('0') << setw(8) << hex << csr[0];
  cout << " Handler: 0x" << setfill('0') << setw(8) << hex << csr[1];
  cout << " Cause: 0x" << setfill('0') << setw(8) << hex << csr[2] << endl;

  cout << "Current instruction code: "<< bytes[regs[15]] + bytes[regs[15] + 1] + bytes[regs[15] + 2] + bytes[regs[15] + 3] << endl;

}

int main(int argc, char* argv[]){

  if(argc != 2){
    cout << "ERROR: Incorect input instruction" << endl;
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

  printf("EMULATOR DONE!!!\n");

  return 1;
}