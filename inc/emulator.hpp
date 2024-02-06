#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <list>

using namespace std;

struct Code{
  int address; 
  string data; 
};

/////////////////KLASA/////////////////
class Emulator{

private:
  static vector<Code> codes;
  static string inputFile;

public:

  static void init(string fileName);
  static void startEmulator();
  static void getTextFile(string fileName);

  static vector<string> splitString(const string& input, char delimiter);
  static void displayCode(const vector<Code>& codeVector);

};

#endif