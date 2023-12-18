#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <list>

//1. napraviti funkcionalan lekser
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

using namespace std;

struct RealocationEntry {
  //u kojoj sekciji se radi realokacija
  string section;
  //offset - ovo je offset od pocetka sekcije gde treba da se upise
  int offset; //mozda da bude long long kao i addent
  //simbol
  string symbol;
  //simbol koji treba da se redefinise zato sto se lokalni simboli ne upisuju u tabelu - addent
  //addent je offset od pocetka sekcija simbola koji se upisuje
  int addent;
  //isData - opciono (ne treba)
};

struct SymbolTableEntry {
  //val
  int value;
  //flag - local or global (mogu i 2 flaga za svaki po jedan)
  bool local;
  bool global;
  //isDefined - opciono (mozda treba samo za equ sekciju)
  //name simbola
  string name;
  //sekcija kojoj pripada
  string section;
  //flag - da li je simbol sekcija ili nije - opciono
  bool isSection;
};

struct PoolOfLiterals{
  //da li je konstanta ili simbol
  bool isSymbol;
  //adresa - vrednost na kojoj se taj simbol nalazi - treba da se cuva od pocetka sekcije, a ne od bazena literala
  int symbolAdress;
  //value simbola
  int symbolValue;
  //string - ime simbola
  int symbolName;
};

struct SectionEntry {
  //velicina
  int size;
  //redni br sekcije
  int serialNum;
  //ime sekcije
  int name;
  //da li ova sekcija ima bazen
  bool hasPool;
  //velicina bazena - sekcija ne sme da ima vise od 2 na 12 bita ako ima bazen
  int poolSize;
  //vektor ovseta <Long Long>
  vector <long long> offsets;
  //vektor data <Char>
  vector <char> data; //ovo moze da se implementira i kao niz
};

/////////////////KLASA/////////////////
class Assembler{
  //vodi racuna o bazenu literala
  //o tome da se ne moze svaka instrukcija isto uraditi zbog bazena literala 
  //(povecavanje bazena literala nakon svake instrukcije je no no)

private:

public:

};

#endif