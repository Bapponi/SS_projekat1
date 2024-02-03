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
vector<RealocationEntry> Linker::relVector;

map<string, map<string, Symbol>> Linker::symbolMaps;
map<string, map<string, vector<RealocationEntry>>> Linker::relocationMaps;
map<string, map<string, Section>> Linker::sectionMaps;

void Linker::init(){
  relocations.clear();
  symbols.clear();
  pools.clear();
  sections.clear();
  relVector.clear();

  relocationMaps.clear();
  symbolMaps.clear();
  sectionMaps.clear();
}

void Linker::getData(string fileName){

  ifstream input(fileName, ios::binary);

  if (input.fail()){
      cout<<"ERROR: There is problem with opening file: "<< fileName <<endl;
      exit(1);
  }

  cout << "USAO nakon proverie ispravnosti fajle"<< endl;

  int numOfSymbols = 0;
  input.read((char *)&numOfSymbols, sizeof(numOfSymbols));
    
  cout << "USAO pre for petlje za simbole, njihov broj: " << numOfSymbols << endl;
  
  for (int i = 0; i < numOfSymbols; i++){
    Symbol s;
    string name;        
    int stringLength;
    input.read((char *)(&stringLength), sizeof(stringLength));
    name.resize(stringLength);
    input.read((char*)name.c_str(), stringLength);
    input.read((char*)(&s.serialNum), sizeof(s.serialNum));
    input.read((char*)(&s.value), sizeof(s.value));
    input.read((char*)(&s.name), sizeof(s.name));
    input.read((char*)(&s.section), sizeof(s.section));
    input.read((char*)(&s.isSection), sizeof(s.isSection));
    input.read((char*)(&s.offset), sizeof(s.offset));

    stringLength;
    input.read((char *)(&stringLength), sizeof(stringLength));
    s.name.resize(stringLength);
    input.read((char*)s.name.c_str(), stringLength);

    stringLength;
    input.read((char *)(&stringLength), sizeof(stringLength));
    s.section.resize(stringLength);
    input.read((char*)s.section.c_str(), stringLength);

    symbols[name]=s;
  }

  cout << "USAO nakon petlje za simbole" << endl;
  
  symbolMaps[fileName]=symbols;

  int numOfSections = 0;
  input.read((char *)&numOfSections, sizeof(numOfSections));

  for (int i=0;i<numOfSections;i++)
  {
      Section sec;
      string name;
      int stringLength;
      input.read((char *)(&stringLength), sizeof(stringLength));
      name.resize(stringLength);
      input.read((char*)name.c_str(), stringLength);
  
      input.read((char*)(&sec.size), sizeof(sec.size));
      input.read((char*)(&sec.serialNum), sizeof(sec.serialNum));
      input.read((char*)(&sec.name), sizeof(sec.name));
      input.read((char*)(&sec.hasPool), sizeof(sec.hasPool));
      input.read((char*)(&sec.poolSize), sizeof(sec.poolSize));

      stringLength;
      input.read((char*)(&stringLength), sizeof(stringLength));
      sec.name.resize(stringLength);
      input.read((char*)sec.name.c_str(), stringLength);
      int numOfRelocations;

      input.read((char *)&numOfRelocations, sizeof(numOfRelocations));

      for (int j=0;j<numOfRelocations;j++){

        RealocationEntry reloc;

        input.read((char*)(&reloc.section),sizeof(reloc.section));
        input.read((char*)(&reloc.offset), sizeof(reloc.offset));
        input.read((char*)(&reloc.symbol), sizeof(reloc.symbol));
        input.read((char*)(&reloc.addent), sizeof(reloc.addent));

        unsigned int stringLength; 
        input.read((char *)(&stringLength), sizeof(stringLength));
        reloc.section.resize(stringLength);
        input.read((char*)reloc.section.c_str(), stringLength);

        stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        reloc.symbol.resize(stringLength);
        input.read((char*)reloc.symbol.c_str(), stringLength);
        relVector.push_back(reloc);
      }
  
      relocationMaps[sec.name][fileName] = relVector;
      int dataSize;
      input.read((char *)&dataSize, sizeof(dataSize));

    
      for (int k = 0; k < dataSize; k++){
          string data;
          input.read((char *)(&data), sizeof(data));
          sec.data.push_back(data);
      }

      int offsetSize; 
      input.read((char*)(&offsetSize), sizeof(offsetSize));
  
      for (int l=0; l < offsetSize; l++){
          long long offset;
          input.read((char *)(&offset), sizeof(offset));
          sec.offsets.push_back(offset);
      }
      sections[name]=sec;
  }

  sectionMaps[fileName]=sections;

  relVector.clear();
  symbols.clear();
  sections.clear();

  input.close();

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
  cout << "USAO pre getData"<< endl;
  Linker::getData(argv[2]);

  printf("Prosao linker bez greske\n");

  return 1;
}