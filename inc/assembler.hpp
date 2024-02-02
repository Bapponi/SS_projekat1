#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <list>

//1. napraviti funkcionalan lekser i parser
//2. baviti se potrebnim strukturama fajla

//razmisli sta sve treba od struktura
//bazen literala (simboli i konstante koje su vece od 12b ili cija se vrednost ne yna u toku asembliranja) 
//na kraju svake sekcije, to je potrebno zbog velicine
//uvodi se ogranicenje da sekcija ne moze da bude veca od 2 na 12 bita
//to je sekcija koja se sastoji od word direktiva
//trebace realokacija
//ako su konstante manje od 12 bita, one se ugradjuju u instrukciju direktno
//ako su vece, onda se stavljaju u bazen literala
//pomeraj do tog literala se ugradjuje u bazen
//predavanja za 2 prolazni asembler - predavanje 5, 6 i 7 do 26 minuta
//vezbe trebaju da se odgledaju zadaci 1, 2, 5, 8, 9  

//treba u parseru da se napravi niz struktura gde ce da se pamte stvari iz parsera
//za stvari poput labela, direktiva, instrukcija, konstanti (razdvojiti sve logicke stvari)
//U akcijama u parseru se dodaju stvari u te nizove i nakon zavrsetka parsiranja se imaju
//popunjene strukture sa kojima moze da se radi u assembler.cpp delu

//vodi racuna o bazenu literala
//o tome da se ne moze svaka instrukcija isto uraditi zbog bazena literala 
//(povecavanje bazena literala nakon svake instrukcije je no no)

//u drugom prolazu se generise kod na osnovu instrukcija
//moguce je da se za jednu naredbu generisu dve instrukcije zbog bazena literala
//generisanje relokacionih zapisa
//u instrukciji se generise odskok od same instrukcije do same instrukcije u bazenu literala
//za drugi prolaz raditi generisanje koda kod instrukcije
//radi u sekcijama locationCounter da je nula
//map<string, vector<PoolOfLiterals>>
//za generisanje koda napravi sheet sta se u sta mapira!!!



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
class Assembler{

private:
  static map<string, vector<RealocationEntry>> relocations;
  static map<string, Symbol> symbols;
  static map<string, vector<PoolOfLiterals>> pools;
  static map<string, Section> sections;
  static vector<PoolOfLiterals> poolVector;
  
  static string fileOutput;
  static string currentSectionName;
  static int instructionNum;
  static int currentSectionSize;
  static string currentDirective;
  static int symSerialNum;
  static int secSerialNum;
  static string currentInstruction;
  static int fileOffset;
  static bool hasPool;
  static int poolOffset;
  static long long skipWordNum;

  static string currentOperandOffset;
  static bool hasPool2;
  static int skipNum;
  static bool inParrens;
  static string parrensReg;
  static string parrensHex;
  static bool inOprString;

public:

  static bool secondPass;
  static void init();
  static void passFile(string fileName, string fileOut, int passNum);

  static void getIdent(string name, bool isGlobal);
  static void startSection(string name);
  static void programEnd();
  static void directiveStart(string name);
  static void directiveEnd();
  static void labelStart(string name);
  static void instructionPass(string name);
  static void getLiteral(string name, string type);
  static void getOperand(string name, string type);
  static void getParrensBody(string name, string type);
  static void instructionName(string name);

  static void instructionPass2(string name, string op1, string op2);
  static void startSection2(string name);
  static void programEnd2();
  static void getOperand2(string name, string type);
  static void getParrensBody2(string name, string type);
  static void getLiteral2(string name, string type);

  static bool inTable(string name);
  static string getBits(const string& stringInt, int nBits);
  static string getOperandOffset();

  static void displaySymbolTable(const map<string, Symbol>& symbolMap);
  static void displaySectionTable(const map<string, Section>& symbolMap);
  static void displayPoolTable(const map<string, vector<PoolOfLiterals>>& symbolMap);
  static void displayRelocationTable(const map<string, vector<RealocationEntry>>& symbolMap);

};

#endif