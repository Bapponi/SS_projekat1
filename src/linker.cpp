#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
#include <iomanip> 

#include "../inc/linker.hpp"

using namespace std;

map<string, vector<RealocationEntry>> Linker::relocations;
map<string, Symbol> Linker::symbols;
map<string, vector<PoolOfLiterals>> Linker::pools;
map<string, Section> Linker::sections;
vector<PoolOfLiterals> Linker::poolVector;

void Linker::init(){
  relocations.clear();
  symbols.clear();
  pools.clear();
  sections.clear();
  poolVector.clear();
}

int main(int argc, char* argv[]){

  if(argc != 3){
    cout << "INPUT ERROR: Nedovaoljan broj argumenata za asembliranje!!!";
    exit(1);
  }
  
  if(strcmp(argv[1], "-o")){
    cout << "INPUT ERROR: lose formatirana funkcija!!!";
    exit(1);
  }
  size_t lengthOutput = strlen(argv[2]);

  char lastTwoOutput[3];
  if (lengthOutput >= 2) {
    strncpy(lastTwoOutput, argv[2] + lengthOutput - 2, 2);
    lastTwoOutput[2] = '\0';
  }else{
    cout << "INPUT ERROR: lose formatirani fajlovi duzina!!!";
    exit(1);
  }

  if(strcmp(lastTwoOutput, ".o")){
    cout << "INPUT ERROR: lose formatirani fajl izgled output!!!";
    exit(1);
  }

  Linker::init();

  printf("Prosao linker bez greske\n");

  return 1;
}