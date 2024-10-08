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
  static vector<unsigned int> regs;
  static vector<unsigned int> csr;

  static unsigned int startAddress;
  static unsigned int regM;
  static int pc;
  static int sp;
  static int status;
  static int handler;
  static int cause;
  static bool executing;
  static bool error;
  static int inst1;
  static int instM;
  static int instA;
  static int instB;
  static int instC;
  static int instD;
  static string variation;
  static int instructionNum;
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
  static long long getValueFromAddress(unsigned int address);
  static string getStringFromAddress(int address);
  static void setValueOnAddress(int address, int value);
  static unsigned int popFromStack();
  static void pushOnStack(int value);
  static void startInterrupt();
  
  static string binaryToHex(const string& binaryString, int size);
  static string decimalToHex(int decimalValue);
  static vector<string> splitString(const string& input, char delimiter);
  static void displayCode(const vector<Code>& codeVector);
  static void showCurrentState();

};

#endif