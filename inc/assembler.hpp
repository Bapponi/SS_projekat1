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

using namespace std;

struct RealocationEntry {
  string section;
  int offset;
  string symbol;
  int addent;
};

struct Symbol {
  int value;
  bool isLocal;
  string name;
  string section;
  bool isSection;
};

struct PoolOfLiterals{
  bool isSymbol;
  int symbolAdress;
  int symbolValue;
  string symbolName;
};

struct Section {
  int size;
  int serialNum;
  int name;
  bool hasPool;
  int poolSize;
  vector <long long> offsets;
  vector <char> data;
};

/////////////////KLASA/////////////////
class Assembler{

private:
  static map<string, RealocationEntry> relocations;
  static map<string, Symbol> symbols;
  static map<string, PoolOfLiterals> pools;
  static map<string, Section> sections;
  static bool secondPass;
  static string currentSection;
  static int instructionNum;
  static int currentSectionSize;
  static string currentDirective;

public:
  static void init();
  static void passFile(string fileName, int fileNum, int passNum);

  static void getIdent(string name, bool isGlobal);
  static void startSection(string name);
  static void directiveStart(string name);
  static void directiveEnd();
  static void labelStart(string name);
  static void instructionPass(string name);
  static void getLiteral(string name);
};

#endif