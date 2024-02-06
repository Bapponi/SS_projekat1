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

void Emulator::init(string fileName){
  codes.clear();
  inputFile = fileName;
}

void Emulator::startEmulator(){
  getTextFile(inputFile);  
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
    c.data = vektor.at(1);
    codes.push_back(c);
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

void Emulator::displayCode(const vector<Code>& codeVector){
  cout << "------------------------------------Codes-------------------------------------" << endl;
  cout << setw(20) << "Address" << setw(36) << "Data" << endl;

  for (int i = 0; i < codeVector.size(); i++) {
    cout << setw(20) << codeVector.at(i).address  << ":" << setw(36) << codeVector.at(i).data << endl;
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