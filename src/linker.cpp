#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
#include <iomanip> 

//za fajl sistem
#include <cstring>
#include <dirent.h>

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
  input.read((char*)&numOfSymbols, sizeof(numOfSymbols));
    
  cout << "USAO pre for petlje za simbole, njihov broj: " << numOfSymbols << endl;
  
  for (int i = 0; i < numOfSymbols; i++){

    cout << "Pocetak for petlje" << endl;
    Symbol s;
    string name;        
    unsigned int nameSize;
    input.read((char*)(&nameSize), sizeof(nameSize));
    cout << "NameSize: " << nameSize << endl;

    name.resize(nameSize);
    cout << "NameSize: " << nameSize << endl;

    input.read((char*)name.c_str(), nameSize);
    cout << "Name.c_str: " << name.c_str() << endl;

    input.read((char*)(&s.serialNum), sizeof(s.serialNum));
    cout << "SerialNum: " << s.serialNum << endl;

    input.read((char*)(&s.value), sizeof(s.value));
    cout << "Value: " << s.value << endl;

    input.read((char*)(&s.name), sizeof(s.name));
    cout << "Name: " << s.name << endl;
    
    input.read((char*)(&s.section), sizeof(s.section));
    cout << "Section: " << s.section << endl;

    input.read((char*)(&s.isSection), sizeof(s.isSection));
    cout << "IsSection: " << s.isSection << endl;
    
    input.read((char*)(&s.offset), sizeof(s.offset));
    cout << "Offset: " << s.offset << endl;

    // cout << "Upis u simbol: " << s.name << endl;
    cout << "Upis u simbol" << endl;

    nameSize; //za reset vrednosti
    input.read((char*)(&nameSize), sizeof(nameSize));
    s.name.resize(nameSize);
    input.read((char*)s.name.c_str(), nameSize);

    nameSize;
    input.read((char *)(&nameSize), sizeof(nameSize));
    s.section.resize(nameSize);
    input.read((char*)s.section.c_str(), nameSize);

    symbols[name]=s;

    cout << "Kraj for petlje" << endl;
  }

  cout << "USAO nakon petlje za simbole" << endl;
  
  symbolMaps[fileName]=symbols;

  int numOfSections = 0;
  input.read((char *)&numOfSections, sizeof(numOfSections));

  for (int i=0;i<numOfSections;i++)
  {
      Section sec;
      string name;
      unsigned stringLength;
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

void Linker::getTextFile(string fileName){

  ifstream file(fileName);
  
  string line;

  while(getline(file, line)){

    if (line == "===") {
      break; 
    }

    int num = stoi(line);
    RealocationEntry entry;
    string section;
    for(int i = 0; i < num; i++ ){

      getline(file, line);

      vector<string> vektor = splitString(line, ',');

      entry.section = vektor.at(0);
      string offset = vektor.at(1);
      entry.offset = stoi(offset);
      entry.symbol = vektor.at(2);
      string addent = vektor.at(3);
      entry.addent = stoi(addent);
      
      section = entry.section;
      relVector.push_back(entry);

    }

    relocations.insert(make_pair(section, relVector));
    relocationMaps.insert(make_pair(fileName, relocations));
    displayRelocationTable(relocations);
    relocations.clear();
  }

  while(getline(file, line)){
    
    if (line == "===") {
      break; 
    }

    Section sec;
    string sectionName;
    vector<string> vektor = splitString(line, ',');
    sec.name = vektor.at(0);
    string size = vektor.at(1);
    sec.size = stoi(size);
    string serialNum = vektor.at(2);
    sec.serialNum = stoi(serialNum);
    string hasPool = vektor.at(3);
    sec.hasPool = stoi(hasPool);
    string poolSize = vektor.at(4);
    sec.poolSize = stoi(poolSize);

    getline(file, line);
    int num = stoi(line);

    getline(file, line);
    vector<string> offsets = splitString(line, ' ');
    for(int i = 0; i < num; i++){
      sec.offsets.push_back(stoi(offsets.at(i)));
    }
    
    getline(file, line);
    vector<string> data = splitString(line, ' ');
    sec.data = data;

    sections.insert(make_pair(sec.name, sec));

  }

  sectionMaps.insert(make_pair(fileName, sections));
  displaySectionTable(sections);
  sections.clear();

  //simboli
  getline(file, line);
  int num = stoi(line);

  for(int i = 0; i < num; i++){
    
    Symbol s;
    getline(file, line);
    vector<string> vektor = splitString(line, ',');

    s.name = vektor.at(0);
    string serialNum = vektor.at(1);
    s.serialNum = stoi(serialNum);
    string value = vektor.at(2);
    s.value = stoi(value);
    string isLocal = vektor.at(3);
    s.isLocal = stoi(isLocal);
    s.section = vektor.at(4);
    string isSection = vektor.at(5);
    s.isSection = stoi(isSection);
    string offset = vektor.at(6);
    s.offset = stoi(offset);

    symbols.insert(make_pair(s.name, s));
  }

  symbolMaps.insert(make_pair(fileName, symbols));
  displaySymbolTable(symbols);
  symbols.clear();
}

//POMOCNE FUNKCIJE//////////////////////////////////

vector<string> Linker::splitString(const string& input, char delimiter) {
  
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

void Linker::displayRelocationTable(const map<string, vector<RealocationEntry>>& symbolMap){

  cout << "    -------------------------RELOCATIONS-----------------------" << std::endl;
  cout << setw(15) << "Section" << setw(15) << "Offset" << setw(15) << "Symbol"
       << setw(15) << "Addent" << endl;

  for (const auto& entry : symbolMap) {
      const vector<RealocationEntry>& relocations = entry.second;

      for (const auto& relocation : relocations) {
        cout << setw(15) << relocation.section << setw(15) << relocation.offset
             << setw(15) << relocation.symbol << setw(15) << relocation.addent << endl;
      }
  }

  cout << "\n" << endl;
}

void Linker::displaySectionTable(const map<string, Section>& symbolMap){
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

void Linker::displaySymbolTable(const map<string, Symbol>& symbolMap){
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

  const char* currentPath = "."; // Specify the current directory

  DIR* dir = opendir(currentPath);
  if (dir != nullptr) {

      struct dirent* entry;
      while ((entry = readdir(dir)) != nullptr) {

          if (entry->d_type == DT_REG && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && std::strstr(entry->d_name, ".txt") != nullptr) {
              cout << "Ime fajla: " << entry->d_name << endl;
              Linker::getTextFile(entry->d_name);
          }

      }
      closedir(dir);

  } else {
      cout << "ERROR opening directories!!!" << endl;
      exit(1);
  }

  printf("Prosao linker bez greske :)\n");

  return 1;
}