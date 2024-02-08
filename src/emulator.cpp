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
vector<int> Emulator::regs;
vector<int> Emulator::csr; 

unsigned int Emulator::regM;
int Emulator::startAddress;
int Emulator::pc;
int Emulator::sp;
int Emulator::status;
int Emulator::handler;
int Emulator::cause;
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
  cout << "In init" << endl;
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

  regM = 0xFFFF0000;
  startAddress = 0;
  pc = 15;
  sp = 14;

  regs[pc] = startAddress;
  regs[sp] = regM;
  csr[handler] = regM;
  executing = true;
  error = false;
  inst1 = "";
  instM = "";
  instA = "";
  instB = "";
  instC = "";
  instD = "";
  variation = "";
  cout << "Out of init" << endl;
}

void Emulator::startEmulator(){
  getTextFile(inputFile);
  displayCode(codes);
  setupBytes();
  programExecute();  
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
    regs[pc] += 4;
  }
}

void Emulator::instructionStart(){

  setInstructionReg();

  if(inst1 == "0"){
  
    currentInstruction = "halt";
    if(instM != "0" || instA != "0" || instB != "0" || instC != "0" || instD != "000"){
      error = true;
    }
    executing = false;
  
  }else if(inst1 == "1"){

    currentInstruction = "int";
    if(instM != "0" || instA != "0" || instB != "0" || instC != "0" || instD != "000"){
      error = true;
    }
    startInterrupt();
  
  }else if(inst1 == "2"){

    currentInstruction = "call";

    if(instM == "0"){
      regs[pc] = regs[stoi(instA)] + regs[stoi(instB)] + stoi(instD);
    }        
    else if(instM == "1"){
      regs[pc] = getValueFromAddress(regs[stoi(instA)] + regs[stoi(instB)] + stoi(instD));
    }
    else error = true;    

    if(instC != "0") error = true;  

  }else if(inst1 == "3"){
    
    if(instM == "0"){
      currentInstruction = "jmp";
      
      regs[pc] = regs[stoi(instA)] + stoi(instD);

      if(instB != "0" || instC != "0") error = true;         
    }
    else if(instM == "1"){
      currentInstruction = "beq";
      
      if(regs[stoi(instB)] == regs[stoi(instC)])
        regs[pc] = regs[stoi(instA)] + stoi(instD);

    }
    else if(instM == "2"){
      currentInstruction = "bne";
      
      if(regs[stoi(instB)] != regs[stoi(instC)])
        regs[pc] = regs[stoi(instA)] + stoi(instD);

    }
    else if(instM == "3"){
      currentInstruction = "bgt";
      
      if(regs[stoi(instB)] > regs[stoi(instC)])
        regs[pc] = regs[stoi(instA)] + stoi(instD);

    }
    else if(instM == "8"){
      currentInstruction = "jmp";
      regs[pc] = getValueFromAddress((regs[stoi(instA)] + stoi(instD)));

      if(instB != "0" || instC != "0") error = true;         
    }
    else if(instM == "9"){
      currentInstruction = "beq";
      
      if(regs[stoi(instB)] == regs[stoi(instC)])
        regs[pc] = getValueFromAddress((regs[stoi(instA)] + stoi(instD)));

    }
    else if(instM == "a"){
      currentInstruction = "bne";
      
      if(regs[stoi(instB)] != regs[stoi(instC)])
        regs[pc] = getValueFromAddress((regs[stoi(instA)] + stoi(instD)));

    }
    else if(instM == "b"){
      currentInstruction = "bgt";
      
      if(regs[stoi(instB)] > regs[stoi(instC)])
        regs[pc] = getValueFromAddress((regs[stoi(instA)] + stoi(instD)));

    }
    else error = true;

  }else if(inst1 == "4"){

    currentInstruction = "xchg";

    int help = regs[stoi(instA)];
    regs[stoi(instA)] = regs[stoi(instB)];
    regs[stoi(instB)] = help;

    if(instM != "0" || instA != "0" || instD != "000"){
      error = true;
    }

  }else if(inst1 == "5"){

    if(instM == "0"){
      currentInstruction = "add";
      regs[stoi(instA)] = regs[stoi(instB)] + regs[stoi(instC)];
    } 
    else if(instM == "1"){
      currentInstruction = "sub";
      regs[stoi(instA)] = regs[stoi(instB)] - regs[stoi(instC)];
    } 
    else if(instM == "2"){
      currentInstruction = "mul";
      regs[stoi(instA)] = regs[stoi(instB)] * regs[stoi(instC)];
    } 
    else if(instM == "3"){
      currentInstruction = "div";

      if(instC != "0")
        regs[stoi(instA)] = regs[stoi(instB)] / regs[stoi(instC)];
      else
        error = true;

    } 
    else error = true;

    if(instD != "000") error = true;

  }else if(inst1 == "6"){
    if(instM == "0"){
      currentInstruction = "not";

      regs[stoi(instA)] = !regs[stoi(instB)];

      if(instC != "0") error = true;
    } 
    else if(instM == "1"){
      currentInstruction = "and";
      regs[stoi(instA)] = regs[stoi(instB)] & regs[stoi(instC)];
    }
    else if(instM == "2"){
      currentInstruction = "or";
      regs[stoi(instA)] = regs[stoi(instB)] | regs[stoi(instC)];
    }
    else if(instM == "3"){
      currentInstruction = "xor";
      regs[stoi(instA)] = regs[stoi(instB)] ^ regs[stoi(instC)];
    }
    else error = true;

    if(instD != "000") error = true;

  }else if(inst1 == "7"){

    if(instM == "0"){
      currentInstruction = "shl";
      regs[stoi(instA)] = regs[stoi(instB)] << regs[stoi(instC)];
    }
    else if(instM == "1"){
      currentInstruction = "shr";
      regs[stoi(instA)] = regs[stoi(instB)] >> regs[stoi(instC)];
    }
    else error = true;

    if(instD != "000") error = true;

  }else if(inst1 == "8"){

    currentInstruction = "st";
    if(instM == "0"){

      int address = regs[stoi(instA)] + regs[stoi(instB)] + stoi(instD);
      int value = regs[stoi(instC)];
      setValueOnAddress(address, value);

    }
    else if(instM == "1"){

      pushOnStack(regs[stoi(instC)]);

    }else if(instM == "2"){

      int address = getValueFromAddress(regs[stoi(instA) + regs[stoi(instB) + stoi(instB)]]);
      int value = regs[stoi(instC)];
      setValueOnAddress(address, value);

    }
    else error = true;

  }else if(inst1 == "9"){

    currentInstruction = "ld";

    if(instM == "0"){
      regs[stoi(instA)] = csr[stoi(instB)];
    }
    else if(instM == "1"){
      if(instA == "e" && instB == "e" && instC == "0" && instD == "008"){
        currentInstruction = "iret";
        regs[pc] += 4;
        string nextInstr = getStringFromAddress(regs[pc]);
        if(nextInstr == "10010110000011100000111111111100"){
          regs[pc] += 4;
          string nextInstr = getStringFromAddress(regs[pc]);
          if(nextInstr == "10010110000011100000111111111100"){

          }else error = true;
        }else error = true;
      }else{
        regs[stoi(instA)] = regs[stoi(instB)] + stoi(instB);
      }
    }
    else if(instM == "2"){

      int address = regs[stoi(instB)] + regs[stoi(instC)] + stoi(instD);
      regs[stoi(instA)] = getValueFromAddress(address);

    }
    else if(instM == "3"){
      if(instA == "f" && instB == "e" && instC == "0" && instD == "004"){
        currentInstruction = "ret";
        popFromStack();
      }else{
        regs[stoi(instA)] = popFromStack();
      }
    }
    else if(instM == "4"){
      csr[stoi(instA)] = regs[stoi(instB)];
    }
    else if(instM == "5"){
      csr[stoi(instA)] = regs[stoi(instB)] | stoi(instD);
    }
    else if(instM == "6"){

      int address = regs[stoi(instB)] + regs[stoi(instC)] + stoi(instD);
      csr[stoi(instA)] = getValueFromAddress(address);

    }
    else if(instM == "7"){

      int address = regs[stoi(instB)];
      csr[stoi(instA)] = getValueFromAddress(address);
      regs[stoi(instB)] = regs[stoi(instB)] + stoi(instD);

    }
    else error = true;

  }else{
    cout << "\nERROR: Bad instruction" << endl;
    error = true;
  }

  showCurrentState();

  if(error){
    cout << "ERROR: something bad with instruction: " << currentInstruction << endl;
    exit(1);
  }
}

void Emulator::setInstructionReg(){
  inst1 = bytes[regs[pc]].at(0);
  instM = bytes[regs[pc]].at(1);
  instA = bytes[regs[pc] + 1].at(0);
  instB = bytes[regs[pc] + 1].at(1);
  instC = bytes[regs[pc] + 2].at(0);
  instD = bytes[regs[pc] + 2].at(1) + bytes[regs[pc] + 3];
}

int Emulator::getValueFromAddress(int address){
  return stoi(bytes[address] + bytes[address + 1] + bytes[address + 2] + bytes[address + 3]); 
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
  regs[sp] = regs[sp] + 4;
  
  return value;
}

void Emulator::pushOnStack(int value){
  regs[sp] = regs[sp] - 4;
  setValueOnAddress(regs[sp], value);
}

void Emulator::startInterrupt(){
  
  if(csr[handler] == regM)
    return;

    pushOnStack(csr[status]);
    pushOnStack(regs[pc]);

    regs[pc] = csr[handler];

    csr[status] = I | TR | TL;
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
  // ss << hex << decimalValue;
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

  cout << "\nTrenutna instrukcija: " << currentInstruction << endl;

  cout << "Izgled: "<< bytes[regs[15]] + bytes[regs[15] + 1] + bytes[regs[15] + 2] + bytes[regs[15] + 3] << endl;

  for(int i = 0; i < regs.size() - 2; i++){
    cout << "REG" << i << ": " << regs[i] << " ";
  }
  cout << "\nSP: "<< regs[14] << " PC: " << regs[15] << endl;
  cout << "Status: " << csr[0] << " Handler: " << csr[1] << " Cause: " << csr[2] << endl;

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