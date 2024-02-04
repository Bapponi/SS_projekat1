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
  int offset;
  string symbol;
  int addent;
};

struct Symbol {
  int serialNum;
  int value;
  bool isLocal;
  string name;
  string section;
  bool isSection;
  int offset;
};

struct PoolOfLiterals{
  bool isSymbol;
  int symbolAddress;
  long long symbolValue;
  string symbolName;
};

struct Section {
  int size;
  int serialNum;
  string name;
  bool hasPool;
  int poolSize;
  vector <long long> offsets;
  vector <string> data;
};

/////////////////KLASA/////////////////
class Linker{

private:
  static map<string, vector<RealocationEntry>> relocations;
  static map<string, Symbol> symbols;
  static map<string, vector<PoolOfLiterals>> pools;
  static map<string, Section> sections;
  static vector<RealocationEntry> relVector;

  static map<string, map<string, Symbol>> symbolMaps;
  static map<string, map<string, vector<RealocationEntry>>> relocationMaps;
  static map<string, map<string, Section>> sectionMaps;

public:

  static void init();
  static void getData(string fileName);
  static void getTextFile(string fileName);

  static vector<string> splitString(const string& input, char delimiter);
  static void displayRelocationTable(const map<string, vector<RealocationEntry>>& symbolMap);
  static void displaySectionTable(const map<string, Section>& symbolMap);
  static void displaySymbolTable(const map<string, Symbol>& symbolMap);
  
};

#endif