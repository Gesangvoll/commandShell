#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
int main() {
  char * env = getenv("PATH");
  std::cout << env << std::endl;
  char * input = strdup("./\  ls a grgg");
  char * delim = " ";
  char * temp;

  for (temp = strsep(&input, delim); temp != NULL; temp = strsep(&input, delim)) {
    std::cout << temp << "+";
  }
}
