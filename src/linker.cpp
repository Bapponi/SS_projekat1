#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <array>
#include <vector> 
#include <iomanip> 
#include <regex>

//za fajl sistem
#include <cstring>
#include <dirent.h>

#include "../inc/linker.hpp"

using namespace std;

map<string, vector<RealocationEntry>> Linker::relocations;
map<string, Symbol> Linker::symbols;
map<string, Section> Linker::sections;
map<string, ConnectedSection> Linker::connectedSections;
map<string,long long> Linker::sectionStart;
vector<RealocationEntry> Linker::relVector;

vector<string> Linker::inputFiles;

map<string, map<string, Symbol>> Linker::symbolMaps;
map<string, map<string, vector<RealocationEntry>>> Linker::relocationMaps;
map<string, map<string, Section>> Linker::sectionMaps;

int Linker::currentSectionNum;
int Linker::currentSymbolNum;
int Linker::currentSectionSize;
long long Linker::maxStartAddress;
string Linker::sectionAddressMax;

void Linker::init(map<string,long long> secStart, long long maxAddressStart, string sectionMaxAddress){
  relocations.clear();
  symbols.clear();
  sections.clear();
  connectedSections.clear();
  relVector.clear();

  relocationMaps.clear();
  symbolMaps.clear();
  sectionMaps.clear();

  currentSectionNum = 1;
  currentSymbolNum = 1;
  currentSectionSize = 0;
  maxStartAddress = 0;

  sectionStart = secStart;
  maxStartAddress = maxAddressStart;
  sectionAddressMax = sectionMaxAddress;
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
    relocations.clear();
    relVector.clear();
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

void Linker::linkerStart(vector<string> files){
  displayRelocationMapTable(relocationMaps);
  displaySymbolMapTable(symbolMaps);
  displaySectionMapTable(sectionMaps);

  inputFiles = files;

  for (const auto& outerMap : sectionMaps) {
    const string& fileName = outerMap.first;
    const map<string, Section>& secMap = outerMap.second;

    for (const auto& innerMap : secMap) {

      const string& secName = innerMap.first;
      const Section& section = innerMap.second;

      ConnectedSection cs;
      cs.size = 0;

      connectedSections[secName] = cs;
    }
  }

  for (const auto& outerMap : sectionMaps) {
    const string& fileName = outerMap.first;
    const map<string, Section>& secMap = outerMap.second;

    for (const auto& innerMap : secMap) {

      const string& secName = innerMap.first;
      const Section& section = innerMap.second;

      vector<RealocationEntry> vr;

      relocations[section.name] = vr;
    }
  }

  sectionConnect();
  relocationConnect();
  symbolConnect();
  changeCodeRelocations();
}

void Linker::sectionConnect(){

  for (const auto& innerMap : sectionStart) {
    cout << "Ime sekcije: " << innerMap.first << " vrednost: " << innerMap.second << endl;
  }

  for(string fileName : inputFiles){
    const map<string, Section>& secMap = sectionMaps[fileName];

    for (const auto& innerMap : secMap) {

      const string& secName = innerMap.first;
      const Section& section = innerMap.second;

      connectedSections[secName].name = secName;
      connectedSections[secName].file = fileName;

      cout << "SecName: " << secName << " start: " << sectionStart[secName] << endl;

      if(sectionStart[secName] > 0){
        connectedSections[secName].addressStart = sectionStart[secName];
      }else{
        connectedSections[secName].addressStart = 0;
      // connectedSections[secName].addressStart = sectionMaps[fileName][secName].size + sectionMaps[fileName][secName].poolSize;
      }
      
      cout << "MAX ADDRESS: " << maxStartAddress << endl;


      //TREBA PROMENA...
      for(int i = 0; i < section.offsets.size(); i++){ 
        if(sectionStart[secName] > 0){
          connectedSections[secName].offsets.push_back(section.offsets.at(i) + connectedSections[secName].addressStart);
        }else{
          connectedSections[secName].offsets.push_back(section.offsets.at(i) + connectedSections[secName].size); //pre je bilo .size
          // connectedSections[secName].addressStart = maxStartAddress + connectedSections[sectionAddressMax].size;
        }
          
        connectedSections[secName].data.push_back(section.data.at(i));
      }
      //...DO OVDE


      for(const auto& symbols : symbolMaps[fileName]){
        Symbol s = symbols.second;
        
        if(!s.isLocal && !s.isSection){ // && s.section != "UND" - izbrisano zbog realokacija
          connectedSections[secName].globalStart.insert(make_pair(s.name, connectedSections[secName].size));
        }
      }
 
      connectedSections[secName].size += sectionMaps[fileName][secName].size + sectionMaps[fileName][secName].poolSize;

    }
  }

  displayConnectedSectionTable(connectedSections);

  currentSectionSize += connectedSections[sectionAddressMax].size;

  for(const auto& conSec : connectedSections){

    string sectionName = conSec.first;
    ConnectedSection cs = conSec.second;

    Section sec;
    sec.name = sectionName;
    sec.size = cs.size;
    sec.hasPool = false;
    sec.poolSize = 0;   
    sec.data = cs.data;
    
    if(sectionStart[sectionName] > 0){
      sec.sectionStart = cs.addressStart;
      sec.offsets = cs.offsets;
    }else{
      sec.sectionStart = currentSectionSize + maxStartAddress;

      for(int i = 0; i < cs.offsets.size(); i++)
        sec.offsets.push_back(cs.offsets.at(i) + currentSectionSize + maxStartAddress);
    }

    if (sectionName == "UND")
      sec.serialNum = 0;
    else
      sec.serialNum = currentSectionNum++;

    sections[sectionName] = sec;
    currentSectionSize += cs.size;

  }

  displaySectionTable(sections);

}

void Linker::relocationConnect(){

  for(string fileName : inputFiles){
    const map<string, vector<RealocationEntry>>& relMap = relocationMaps[fileName];

    for (const auto& innerMap : relMap) {
      const string& secName = innerMap.first;
      const vector<RealocationEntry>& relVector = innerMap.second;

      for(int i = 0; i < relVector.size(); i++){

        RealocationEntry re = relVector.at(i);
        //pocetak te sekcije + pocetak te instance sekcije
        if(re.section == re.symbol){
          re.addent += sections[secName].sectionStart; //potencijalno je problem ovde
        }

        //potencijalno problem ovde na desnom delu
        re.offset += sections[secName].sectionStart + connectedSections[secName].globalStart[relVector.at(i).symbol];
        relocations[secName].push_back(re);

      }
    }
  }

  displayRelocationTable(relocations);
}

void Linker::symbolConnect(){  //OVDE IZMENITI VREDNOST SIMBOLA ZBOG DATA DELA!!!!!!!!!!!!!!!

  for(const auto& section : sections) {

    string secName = section.first;
    Section sec = section.second;

    Symbol s;
    s.serialNum = currentSymbolNum;
    s.value = 0;
    s.isLocal = true;
    s.name = sec.name;
    s.section = secName;
    s.isSection = true;
    s.offset = 0;

    symbols[secName] = s;
    currentSymbolNum++;
  }

  for(const auto& outerMap : symbolMaps) {
    string fileName = outerMap.first;
    map<string, Symbol> symMap = outerMap.second;

    for(const auto& innerMap : symMap) {
      string symName = innerMap.first;
      Symbol s = innerMap.second;

      if(!s.isLocal && !s.isSection && s.section != "UND"){
        map<string,Symbol>::iterator itSym=symbols.find(s.name);

        if(itSym == symbols.end()){
          Symbol s2;
          s2.serialNum = currentSymbolNum++;
          //njegova stara vrednost + vrednost pocetka sekcije + pocetak te instance sekcije
          s2.value = symbolMaps[fileName][symName].value + sections[s.section].sectionStart + connectedSections[s.section].globalStart[s.name];
          s2.isLocal = s.isLocal;
          s2.name = s.name;
          s2.section = s.section;
          s2.isSection = false;
          s2.offset = 0;

          symbols[s2.name] = s2;
        }
      }
    }
  }

  displaySymbolTable(symbols);

}

void Linker::changeCodeRelocations(){

  for(const auto& relocation : relocations){
    string secName = relocation.first;
    vector<RealocationEntry> relVector = relocation.second;

    for(int i = 0; i < relVector.size(); i++){
      RealocationEntry re = relVector.at(i);
      long long value = re.addent;
      value += symbols[re.symbol].value;
      long long start = sections[secName].sectionStart;
      string stringVal = getBits(to_string(value), 32);
      long long index = (re.offset - start) / 4;
      sections[secName].data.at(index) = stringVal;
    }
  }

  displaySectionTable(sections);

}

void Linker::makeOutputFile(string fileName){

  ofstream file(fileName, ios::out | ios::binary);
  for (const auto& entry : sections) {
    string secName = entry.first;
    const Section& sec = entry.second;

    for (size_t i = 0; i < sec.offsets.size(); i++) {
      string hexAddress = decimalToHex(sec.offsets.at(i));
      file << setw(8) << setfill('0') << uppercase << hex << sec.offsets.at(i) << "," 
           << sec.data.at(i) << "," 
           << dec << (sec.data.at(i).length() / 8) << "," 
           << hexAddress << '\n';
    }
  }

  file << "===\n";
  file.close();
  makeTextFile(fileName);

}

void Linker::makeTextFile(string fileName){

  fileName.erase(fileName.size() - 4);

  ofstream file(fileName + ".txt");

  for (const auto& entry : sections) {
    string secName = entry.first;
    Section sec = entry.second;

    for(int i = 0; i < sec.offsets.size(); i++){
      string hexAddress = decimalToHex(sec.offsets.at(i));
      file << sec.offsets.at(i) << "," << sec.data.at(i) << "," << sec.data.at(i).length() / 8 << "," << hexAddress << '\n';
    }

  }

  file << "===\n";
  file.close();
}

//////////////////////////POMOCNE FUNKCIJE//////////////////////////////////

string Linker::decimalToHex(int decimalValue) {
  
  stringstream ss;
  ss << hex << setw(8) << setfill('0') << decimalValue;
  return ss.str();

}

string Linker::getBits(const string& stringInt, int nBits) {
    unsigned long intValue = stoul(stringInt);
    
    if (intValue >= (1 << nBits-1)) {
        cout << "ERROR: Integer value exceeds " << nBits << " bits representation." << endl;
        exit(1);
    }

    bitset<32> bits(intValue);
    string bitString = bits.to_string();

    return bitString.substr(32 - nBits);
}

string Linker::binaryToHex(const string& binaryString, int size) {

  bitset<32> bitset(binaryString);
  unsigned long hexValue = bitset.to_ulong();

  stringstream ss;
  ss << hex << setw(size*2) << setfill('0') << hexValue;
  return ss.str();

}

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

  cout << "    ---------------------------------------RELOCATIONS--------------------------------------" << std::endl;
  cout << setw(15) << "Section" 
       << setw(15) << "Offset"
       << setw(20) << "Symbol"
       << setw(15) << "Addent"
       << setw(15) << "Offset HEX"  
       << setw(15) << "Addent HEX" << endl;

  for (const auto& entry : symbolMap) {
      const vector<RealocationEntry>& relocations = entry.second;

      for (const auto& relocation : relocations) {
        cout << setw(15) << relocation.section 
             << setw(15) << relocation.offset
             << setw(20) << relocation.symbol 
             << setw(15) << relocation.addent
             << setw(15) << hex << relocation.offset 
             << setw(15) << relocation.addent << dec << endl;
      }
  }

  cout << "\n" << endl;
}

void Linker::displaySectionTable(const map<string, Section>& symbolMap){
  cout << "       -----------------------------------------------SECTIONS-------------------------------------------" << endl;
  cout << setw(15) << "Name" 
       << setw(10) << "SerialNum" 
       << setw(10) << "SizeAll"
       << setw(15) << "SizeAll HEX"
       << setw(10) << "HasPool" 
       << setw(15) << "PoolSize" 
       << setw(20) << "SectionStart"
       << setw(20) << "SectionStart HEX" << endl;

  for (const auto& entry : sections) {
      const Section& section = entry.second;

      cout << setw(15) << section.name 
           << setw(10) << section.serialNum
           << setw(10) << section.size
           << setw(15) << hex << section.size << dec  
           << setw(10) << section.hasPool
           << setw(15) << section.poolSize 
           << setw(20) << section.sectionStart
           << setw(20) << hex << section.sectionStart << dec <<endl;
  }

  cout << "\n" << endl;

  cout << setw(15) << "Section" 
       << setw(20) << "Offsets" 
       << setw(36) << "Data"
       << setw(15) << "Offsets HEX" 
       << setw(16) << "Data HEX" << endl;

  for (const auto& entry : symbolMap) {
      const vector<long long>& offsetsVector = entry.second.offsets;
      const vector<string>& dataVector = entry.second.data;

      for (int i = 0; i < offsetsVector.size(); i++) {
          cout << setw(15) << entry.first 
               << setw(20) << offsetsVector.at(i) 
               << setw(36) << dataVector.at(i)
               << setw(15) << hex << offsetsVector.at(i) << dec
               << setw(16) << binaryToHex(dataVector.at(i), 4)<< endl;
      }
  }

  cout << "\n" << endl;
}

void Linker::displayConnectedSectionTable(const map<string, ConnectedSection>& symbolMap){
  cout << setw(60) << "    ------------------------CONNECTED SECTIONS----------------------------------------------------" << endl;
  cout << setw(20) << "FILE" 
       << setw(15) << "NAME" 
       << setw(10) << "SIZE"
       << setw(10) << "SIZE HEX"
       << setw(15) << "AddressStart HEX" << endl;

  for (const auto& entry : connectedSections) {
      const ConnectedSection& section = entry.second;

      cout << setw(20) << section.file 
           << setw(15) << section.name
           << setw(10) << section.size
           << setw(10) << hex << section.size 
           << setw(15) << section.addressStart << dec << endl;
  }

  cout << "\n" << endl;

  cout << setw(15) << "Section" 
       << setw(15) << "Offsets" 
       << setw(36) << "Data"
       << setw(15) << "Offsets HEX" 
       << setw(16) << "Data HEX" << endl;

  for (const auto& entry : symbolMap) {
      const vector<long long>& offsetsVector = entry.second.offsets;
      const vector<string>& dataVector = entry.second.data;

      for (int i = 0; i < offsetsVector.size(); i++) {
          cout << setw(15) << entry.first 
               << setw(15) << offsetsVector.at(i) 
               << setw(36) << dataVector.at(i)
               << setw(15) << hex << offsetsVector.at(i) << dec
               << setw(16) << binaryToHex(dataVector.at(i), 4)<< endl;
      }
  }

  cout << "\n" << endl;
}

void Linker::displaySymbolTable(const map<string, Symbol>& symbolMap){
  cout << "         --------------------------------------SYMBOLS-------------------------------------------" << endl;
  cout << setw(15) << "Name" 
       << setw(10) << "SerialNum" 
       << setw(15) << "Value"
       << setw(15) << "Value HEX"
       << setw(10) << "IsLocal" 
       << setw(15) << "Section" 
       << setw(10) << "IsSection"
       << setw(10) << "Offset" << endl;
  
  for (const auto& entry : symbolMap) {
      const Symbol& symbol = entry.second;
      cout << setw(15) << symbol.name 
           << setw(10) << symbol.serialNum
           << setw(15) << symbol.value
           << setw(15) << hex << symbol.value << dec 
           << setw(10) << symbol.isLocal
           << setw(15) << symbol.section 
           << setw(10) << symbol.isSection
           << setw(10) << symbol.offset << endl;
  }

  cout << "\n" << endl;
}

void Linker::displaySymbolMapTable(const map<string, map<string, Symbol>>& symbolMap) {
  cout << setw(110) << "--------------------------------------SYMBOL MAP-----------------------------------------------" << endl;
  cout << setw(20) << "FILE" 
       << setw(20) << "Name" 
       << setw(10) << "SerialNum" 
       << setw(10) << "Value"
       << setw(10) << "Value HEX"
       << setw(10) << "IsLocal" 
       << setw(15) << "Section" 
       << setw(10) << "IsSection"
       << setw(10) << "Offset" << endl;

  for (const auto& outerEntry : symbolMap) {
      const string& fileName = outerEntry.first;
      const map<string, Symbol>& innerMap = outerEntry.second;

      for (const auto& innerEntry : innerMap) {
          const Symbol& symbol = innerEntry.second;
          cout << setw(20) << fileName 
               << setw(20) << symbol.name 
               << setw(10) << symbol.serialNum
               << setw(10) << symbol.value 
               << setw(10) << hex << symbol.value << dec 
               << setw(10) << symbol.isLocal
               << setw(15) << symbol.section 
               << setw(10) << symbol.isSection
               << setw(10) << symbol.offset << endl;
      }
      cout  << setw(110) <<"------------------------------------------------------------------------------------------" << endl;
  }

  cout << "\n" << endl;
}

void Linker::displayRelocationMapTable(const map<string, map<string, vector<RealocationEntry>>>& symbolMap) {
    cout << setw(110) << "--------------------------------------RELOCATION MAP----------------------------------------" << endl;
    cout << setw(20) << "FILE" 
         << setw(15) << "Section" 
         << setw(15) << "Offset" 
         << setw(15) << "Offset HEX"
         << setw(15) << "Symbol"
         << setw(15) << "Addent"
         << setw(15) << "Addent HEX" << endl;

    for (const auto& outerEntry : symbolMap) {
        const string& fileName = outerEntry.first;
        const map<string, vector<RealocationEntry>>& innerMap = outerEntry.second;

        for (const auto& innerEntry : innerMap) {
            const string& symbolName = innerEntry.first;
            const vector<RealocationEntry>& relocations = innerEntry.second;

            for (const auto& relocation : relocations) {
                cout << setw(20)<< fileName 
                     << setw(15) << relocation.section 
                     << setw(15) << relocation.offset
                     << setw(15) << hex << relocation.offset << dec
                     << setw(15) << relocation.symbol 
                     << setw(15) << relocation.addent
                     << setw(15) << hex << relocation.addent << dec << endl;
            }
        }
        cout  << setw(110) <<"----------------------------------------------------------------------------------" << endl;
    }

    cout << "\n" << endl;
}

void Linker::displaySectionMapTable(const map<string, map<string, Section>>& symbolMap){
  cout << setw(105) <<"------------------------------------------------SECTION MAP-----------------------------------------------" << endl;
  cout << setw(20) << "FILE" 
       << setw(15) << "Name" 
       << setw(10) << "SerialNum" 
       << setw(10) << "Size"
       << setw(10) << "Size HEX"
       << setw(10) << "HasPool" 
       << setw(15) << "PoolSize"
       << setw(15) << "PoolSize HEX" << endl;

  for (const auto& outerEntry : symbolMap) {
    const string& sectionName = outerEntry.first;
    const map<string, Section>& innerMap = outerEntry.second;

    for (const auto& innerEntry : innerMap) {
      const Section& section = innerEntry.second;
      cout << setw(20) << sectionName 
           << setw(15) << section.name 
           << setw(10) << section.serialNum
           << setw(10) << section.size
           << setw(10) << hex << section.size << dec 
           << setw(10) << section.hasPool
           << setw(15) << section.poolSize
           << setw(15) << hex << section.poolSize << dec << endl;
    }

    cout << setw(105) << "----------------------------------------------------------------------------------------------------------" << endl;
  }

  cout << "\n" << endl;

  cout << setw(20) << "Section" 
       << setw(10) << "Offsets" 
       << setw(36) << "Data"
       << setw(15) << "Offsets HEX" 
       << setw(16) << "Data HEX" << endl;

  for (const auto& outerEntry : symbolMap) {
    const string& sectionName = outerEntry.first;
    const map<string, Section>& innerMap = outerEntry.second;

    for (const auto& innerEntry : innerMap) {
      const Section& section = innerEntry.second;
      const vector<long long>& offsetsVector = section.offsets;
      const vector<string>& dataVector = section.data;

      for (int i = 0; i < offsetsVector.size(); i++) {
          cout << setw(20) << sectionName 
               << setw(10) << offsetsVector.at(i) 
               << setw(36) << dataVector.at(i)
               << setw(15) << hex << offsetsVector.at(i) << dec
               << setw(16) << binaryToHex(dataVector.at(i), 4)<< endl;
      }
    }

    cout  << setw(97) <<"---------------------------------------------------------------------------------------------" << endl;
  }

  cout << "\n" << endl;
}

int main(int argc, char* argv[]){

  string fileOutput;
  map<string,long long> sectionStart;
  regex inputReg("\\.o$");
  regex outputReg("\\.hex$");
  regex startReg("^-place=([a-zA-Z_][a-zA-Z_0-9]*)@(0[xX][0-9a-fA-F]+)$");
  bool isHex = false;
  vector<string> files;
  long long maxStartAddress = 0;
  string sectionMaxAddress;

  for (int i = 1; i < argc; i++){

    string arg = argv[i];
    
    if(regex_search(argv[i], inputReg)){
      files.push_back(argv[i]);
    }else if(arg == "-hex"){
      isHex = true;
    }else if(regex_search(argv[i], outputReg)){
      fileOutput = argv[i];
    }else if(arg == "-o"){
      isHex = true;
    }else if(regex_search(argv[i], startReg)){

      string all = argv[i];
      vector<string> vektor = Linker::splitString(all, '@');
      long long hex = stoll(vektor.at(1), nullptr, 16);

      vektor = Linker::splitString(vektor.at(0), '=');
      string section = vektor.at(1);
      sectionStart[section] = hex;
      if(hex > maxStartAddress){
        maxStartAddress = hex;
        sectionMaxAddress = section;
      }
          
    }else{
      cout << "ERROR: Bad input: " << argv[i] << endl;
      exit(1);
    }

  }

  Linker::init(sectionStart, maxStartAddress, sectionMaxAddress);
  
  for(int i = 0; i < files.size(); i++){
    cout << files.at(i) << endl;
    files.at(i).erase(files.at(i).end() - 2, files.at(i).end());
    files.at(i) +=  ".txt";
    Linker::getTextFile(files.at(i));
  }

  Linker::linkerStart(files);

  if(isHex)
    Linker::makeOutputFile(fileOutput);
  

  printf("Prosao linker bez greske :)\n");

  return 1;
}