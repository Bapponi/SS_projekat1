#ifndef LINKER_HPP
#define LINKER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <list>

using namespace std;

struct RealocationEntry {
  string section;
  unsigned int offset;
  string symbol;
  unsigned int addent;
};

struct Symbol {
  int serialNum;
  unsigned int value;
  bool isLocal;
  string name;
  string section;
  bool isSection;
  int offset;
};

struct Section {
  int size;
  int serialNum;
  string name;
  bool hasPool;
  int poolSize;
  unsigned int sectionStart;
  vector <long long> offsets;
  vector <string> data;
};

struct ConnectedSection{
  string file;
  string name;
  int size;
  unsigned int addressStart;
  vector <long long> offsets;
  vector <string> data;
  map<string, int> globalStart;
};

/////////////////KLASA/////////////////
class Linker{

private:
  static map<string, vector<RealocationEntry>> relocations;
  static map<string, Symbol> symbols;
  static map<string, Section> sections;
  static map<string, ConnectedSection> connectedSections;
  static map<string, long long> sectionStart;
  
  static vector<RealocationEntry> relVector;
  static vector<string> inputFiles;

  static map<string, map<string, Symbol>> symbolMaps;
  static map<string, map<string, vector<RealocationEntry>>> relocationMaps;
  static map<string, map<string, Section>> sectionMaps;

  static int currentSectionNum;
  static int currentSymbolNum;
  static int currentSectionSize;

public:

  static void init(map<string,long long> secStart);
  static void getTextFile(string fileName);
  static void linkerStart(vector<string>);
  static void sectionConnect();
  static void makeOutputFile(string fileName);
  static void makeTextFile(string fileName);
  static string decimalToHex(int decimalValue);


  static string getBits(const string& stringInt, int nBits);
  static string binaryToHex(const string& binaryString, int size);
  static vector<string> splitString(const string& input, char delimiter);
  static void displayRelocationTable(const map<string, vector<RealocationEntry>>& symbolMap);
  static void displaySectionTable(const map<string, Section>& symbolMap);
  static void displayConnectedSectionTable(const map<string, ConnectedSection>& symbolMap);
  static void displaySymbolTable(const map<string, Symbol>& symbolMap);
  static void displayRelocationMapTable(const map<string, map<string, vector<RealocationEntry>>>& symbolMap); 
  static void displaySymbolMapTable(const map<string, map<string, Symbol>>& symbolMap);
  static void displaySectionMapTable(const map<string, map<string, Section>>& symbolMap);
  static void symbolConnect();
  static void relocationConnect();
  static void changeCodeRelocations();

};

#endif