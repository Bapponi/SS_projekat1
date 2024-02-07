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
  string addressHex; 
  string data;
  string dataHex; 
  int size;
};

/////////////////KLASA/////////////////
class Emulator{

private:
  static vector<Code> codes;
  static string inputFile;
  static string currentInstruction;
  static map<int, string> bytes;
  static int pc;
  static int sp;
  static bool executing;
  static bool error;
  static string inst1;
  static string instM;
  static string instA;
  static string instB;
  static string instC;
  static string instD;
  static string variation;

public:

  static void init(string fileName);
  static void startEmulator();
  static void getTextFile(string fileName);
  static void setupBytes();

  static vector<string> splitString(const string& input, char delimiter);
  static void displayCode(const vector<Code>& codeVector);

  static void programExecute();
  static void executeInstruction();
  static void instructionStart();
  static void setInstructionReg();
  static string binaryToHex(const string& binaryString, int size);
  static string decimalToHex(int decimalValue);


};

#endif