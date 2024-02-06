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

struct Section {
  int size;
  int serialNum;
  string name;
  bool hasPool;
  int poolSize;
  int sectionStart;
  vector <long long> offsets;
  vector <string> data;
};

struct ConnectedSection{
  string file;
  string name;
  int size;
  int addressStart;
  vector <long long> offsets;
  vector <string> data;
};

/////////////////KLASA/////////////////
class Linker{

private:
  static map<string, vector<RealocationEntry>> relocations;
  static map<string, Symbol> symbols;
  static map<string, Section> sections;
  static map<string, ConnectedSection> connectedSections;
  static vector<RealocationEntry> relVector;

  static vector<string> inputFiles;


  static map<string, map<string, Symbol>> symbolMaps;
  static map<string, map<string, vector<RealocationEntry>>> relocationMaps;
  static map<string, map<string, Section>> sectionMaps;

  static int currentSectionNum;
  static int currentSymbolNum;
  static int currentSectionSize;

public:

  static void init();
  static void getTextFile(string fileName);
  static void linkerStart();
  static void sectionConnect();
  static void makeOutputFile(string fileName);

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

};

#endif