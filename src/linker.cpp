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
map<string, Section> Linker::sections;
map<string, ConnectedSection> Linker::connectedSections;
vector<RealocationEntry> Linker::relVector;

map<string, map<string, Symbol>> Linker::symbolMaps;
map<string, map<string, vector<RealocationEntry>>> Linker::relocationMaps;
map<string, map<string, Section>> Linker::sectionMaps;

map<string, int> Linker::sectionEnds;

void Linker::init(){
  relocations.clear();
  symbols.clear();
  sections.clear();
  connectedSections.clear();
  relVector.clear();

  relocationMaps.clear();
  symbolMaps.clear();
  sectionMaps.clear();

  sectionEnds.clear();
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
    // displayRelocationTable(relocations);
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
  symbols.clear();
}

void Linker::linkerStart(){
  displayRelocationMapTable(relocationMaps);
  displaySymbolMapTable(symbolMaps);
  displaySectionMapTable(sectionMaps);

  for (const auto& outerMap : sectionMaps) {
    const string& fileName = outerMap.first;
    const map<string, Section>& secMap = outerMap.second;

    for (const auto& innerMap : secMap) {

      const string& secName = innerMap.first;
      const Section& section = innerMap.second;

      sectionEnds[section.name] = 0;
    }
  }

  sectionConnect();
}

void Linker::sectionConnect(){

  int num = 1;

  for(const auto& outerMap : sectionMaps) {
    const std::string& fileName = outerMap.first;
    const std::map<string, Section>& secMap = outerMap.second;

    cout << "FileName: " << fileName << endl;

    for (const auto& innerMap : secMap) {

      const string& secName = innerMap.first;
      const Section& section = innerMap.second;

      if(secName != "ABS" && secName != "UND"){

        ConnectedSection cs;

        cs.file = fileName;
        cs.name = secName;
        cs.addressStart = sectionEnds[secName];

        if(section.hasPool){
          cs.size = section.offsets.at(section.offsets.size() - 1) + 4;
        }else{
          cs.size = section.size;
        }

        sectionEnds[secName] += section.size;
        connectedSections[secName] = cs; //gledati da li da se i filenameovi racunaju
      }
    }
  }

  for(const auto& secEnd : sectionEnds){

    string sectionName = secEnd.first;
    int sectionEnd = secEnd.second;

    Section sec;
    sec.name = sectionName;
    sec.size = sectionEnd;
    sec.hasPool = false; //voditi racuna o ovome
    sec.poolSize = 0;    //voditi racuna o ovome

    if (sectionName == "ABS")
      sec.serialNum = -1;
    else if (sectionName == "UND")
      sec.serialNum = 0;
    else
      sec.serialNum = num;

    sections[sectionName] = sec;

    num++;

  }

  displaySectionTable(sections);

}

void Linker::makeOutputFile(string fileName){
  ofstream file(fileName, ios::out | ios::binary);
  file.close();
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

void Linker::displaySymbolMapTable(const map<string, map<string, Symbol>>& symbolMap) {
  cout << setw(100) << "-------------------------------------SYMBOLS---------------------------------------------" << endl;
  cout << setw(20) << "FILE" << setw(20) << "Name" << setw(10) << "SerialNum" << setw(10) << "Value"
       << setw(10) << "IsLocal" << setw(15) << "Section" << setw(10) << "IsSection"
       << setw(10) << "Offset" << endl;

  for (const auto& outerEntry : symbolMap) {
      const string& sectionName = outerEntry.first;
      const map<string, Symbol>& innerMap = outerEntry.second;

      for (const auto& innerEntry : innerMap) {
          const Symbol& symbol = innerEntry.second;
          cout << setw(20) << sectionName << setw(20) << symbol.name << setw(10) << symbol.serialNum
               << setw(10) << symbol.value << setw(10) << symbol.isLocal
               << setw(15) << sectionName << setw(10) << symbol.isSection
               << setw(10) << symbol.offset << endl;
      }
      cout  << setw(100) <<"------------------------------------------------------------------------------------------" << endl;
  }

  cout << "\n" << endl;
}

void Linker::displayRelocationMapTable(const map<string, map<string, vector<RealocationEntry>>>& symbolMap) {
    cout << setw(80) << "-----------------------------RELOCATIONS-------------------------------" << endl;
    cout << setw(20) << "FILE" << setw(15) << "Section" << setw(15) << "Offset" << setw(15) << "Symbol"
         << setw(15) << "Addent" << endl;

    for (const auto& outerEntry : symbolMap) {
        const string& fileName = outerEntry.first;
        const map<string, vector<RealocationEntry>>& innerMap = outerEntry.second;

        for (const auto& innerEntry : innerMap) {
            const string& symbolName = innerEntry.first;
            const vector<RealocationEntry>& relocations = innerEntry.second;

            for (const auto& relocation : relocations) {
                cout << setw(20)<< fileName << setw(15) << relocation.section << setw(15) << relocation.offset
                     << setw(15) << symbolName << setw(15) << relocation.addent << endl;
            }
        }
        cout  << setw(80) <<"-------------------------------------------------------------------" << endl;
    }

    cout << "\n" << endl;
}

void Linker::displaySectionMapTable(const map<string, map<string, Section>>& symbolMap){
  cout << setw(80) <<"----------------------------------SECTIONS----------------------------------" << endl;
  cout << setw(20) << "FILE" <<setw(15) << "Name" << setw(10) << "SerialNum" << setw(10) << "Size"
       << setw(10) << "HasPool" << setw(15) << "PoolSize" << endl;

  for (const auto& outerEntry : symbolMap) {
    const string& sectionName = outerEntry.first;
    const map<string, Section>& innerMap = outerEntry.second;

    for (const auto& innerEntry : innerMap) {
      const Section& section = innerEntry.second;
      cout << setw(20) << sectionName << setw(15) << section.name << setw(10) << section.serialNum
           << setw(10) << section.size << setw(10) << section.hasPool
           << setw(15) << section.poolSize << endl;
    }

    cout  << setw(80) << "----------------------------------------------------------------------------" << endl;
  }

  cout << "\n" << endl;

  cout << setw(20) << "Section" << setw(20) << "Offsets" << setw(36) << "Data" << endl;

  for (const auto& outerEntry : symbolMap) {
    const string& sectionName = outerEntry.first;
    const map<string, Section>& innerMap = outerEntry.second;

    for (const auto& innerEntry : innerMap) {
      const Section& section = innerEntry.second;
      const vector<long long>& offsetsVector = section.offsets;
      const vector<string>& dataVector = section.data;

      for (int i = 0; i < offsetsVector.size(); i++) {
          cout << setw(20) << sectionName << setw(20) << offsetsVector.at(i) << setw(36) << dataVector.at(i) << endl;
      }
    }

    cout  << setw(76) <<"------------------------------------------------------------------------" << endl;
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

  const char* currentPath = "."; 

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

  Linker::linkerStart();

  Linker::makeOutputFile(argv[2]);

  printf("Prosao linker bez greske :)\n");

  return 1;
}