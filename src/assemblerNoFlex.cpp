#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void processLine(char *line){
  printf("%s", line);
  char firstWord[20];
  int i = 0;
  while (line[i] != ' ' && line[i] != '\0') {
    firstWord[i] = line[i];
    i++;
  }
  cout << firstWord;
}

int main(int argc, char* argv[]){
  FILE *file = fopen("./test/nivo-a/main.s", "r");
  char line[100];

  if ((!file)) {
    printf("I can't open the file!\n");
    return -1;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    processLine(line);
  }
  
  fclose(file);

  printf("Prosao ceo fajl bez greske");

  return 1;
}