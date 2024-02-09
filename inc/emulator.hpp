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
  long long address;
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
  static map<long long, string> bytes;
  static vector<int> regs;
  static vector<int> csr;

  static unsigned int startAddress;
  static unsigned int regM;
  static int pc;
  static int sp;
  static int status;
  static int handler;
  static int cause;
  static bool executing;
  static bool error;
  static string inst1;
  static string instM;
  static string instA;
  static string instB;
  static string instC;
  static string instD;
  static string variation;
  static int TR;
  static int TL;
  static int I;

public:

  static void init(string fileName);
  static void startEmulator();
  static void getTextFile(string fileName);
  static void setupBytes();

  static void programExecute();
  static void instructionStart();
  static void setInstructionReg();
  static int getValueFromAddress(int address);
  static string getStringFromAddress(int address);
  static void setValueOnAddress(int address, int value);
  static int popFromStack();
  static void pushOnStack(int value);
  static void startInterrupt();
  
  static string binaryToHex(const string& binaryString, int size);
  static string decimalToHex(int decimalValue);
  static vector<string> splitString(const string& input, char delimiter);
  static void displayCode(const vector<Code>& codeVector);
  static void showCurrentState();

};

#endif