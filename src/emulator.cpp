#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
#include <iomanip> 

#include "../inc/emulator.hpp"

using namespace std;

void Emulator::init(){

}

void Emulator::startEmulator(){

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
    
  Emulator::init();
  Emulator::startEmulator();

  printf("Prosao emulator bez greske :)\n");

  return 1;
}